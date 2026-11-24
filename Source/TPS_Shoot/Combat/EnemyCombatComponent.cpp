// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/EnemyCombatComponent.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Combat/EnemyCombatConfig.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "EnemyBase.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TPS_Shoot.h"
#include "TPS_ShootCharacter.h"

UEnemyCombatComponent::UEnemyCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UEnemyCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEnemyCombatComponent, bIsAttacking);
}

void UEnemyCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearAttackTimers();
	ReleaseAttackFocus();
	Super::EndPlay(EndPlayReason);
}

float UEnemyCombatComponent::GetMaxAttackRange() const
{
	return IsValid(CombatConfig) ? CombatConfig->GetMaxAttackRange() : 0.0f;
}

bool UEnemyCombatComponent::IsOnCooldown() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	return World->GetTimeSeconds() < NextAllowedAttackTime;
}

bool UEnemyCombatComponent::CanStartAttack(AActor* TargetActor) const
{
	const AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	if (!IsValid(OwnerEnemy) || !OwnerEnemy->HasAuthority() || OwnerEnemy->IsDead())
	{
		return false;
	}

	if (!IsValid(CombatConfig) || CombatConfig->Attacks.IsEmpty())
	{
		return false;
	}

	if (bIsAttacking || IsOnCooldown())
	{
		return false;
	}

	if (!IsValidAttackTarget(TargetActor))
	{
		return false;
	}

	if (const UCharacterMovementComponent* Movement = OwnerEnemy->GetCharacterMovement())
	{
		if (Movement->IsFalling())
		{
			return false;
		}
	}

	const float DistanceToTarget = OwnerEnemy->GetDistanceTo(TargetActor);
	return DistanceToTarget <= CombatConfig->GetMaxAttackRange();
}

bool UEnemyCombatComponent::TryStartAttack(AActor* TargetActor)
{
	if (!CanStartAttack(TargetActor))
	{
		return false;
	}

	AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	UWorld* World = GetWorld();
	if (!IsValid(OwnerEnemy) || !World)
	{
		return false;
	}

	const float DistanceToTarget = OwnerEnemy->GetDistanceTo(TargetActor);
	const int32 SelectedIndex = CombatConfig->SelectAttackIndex(DistanceToTarget, LastAttackIndex);
	if (!CombatConfig->Attacks.IsValidIndex(SelectedIndex))
	{
		return false;
	}

	const FEnemyAttackSpec& Spec = CombatConfig->Attacks[SelectedIndex];
	const float WindupTime = FMath::Max(0.05f, Spec.WindupTime);
	const float RecoveryTime = FMath::Max(0.05f, Spec.RecoveryTime);
	const float TotalDuration = WindupTime + RecoveryTime;

	// 初始化本轮攻击权威状态
	++CurrentAttackId;
	const uint32 LaunchedAttackId = CurrentAttackId;

	bIsAttacking = true;
	ActiveAttackIndex = SelectedIndex;
	LastAttackIndex = SelectedIndex;
	LockedTarget = TargetActor;
	bHitAttempted = false;

	// 前摇期间允许朝向目标，进入挥击判定后释放焦点
	if (CombatConfig->bFaceTargetDuringWindup)
	{
		if (AAIController* AIController = GetOwnerAIController())
		{
			AIController->SetFocus(TargetActor, EAIFocusPriority::Gameplay);
			bHasAppliedAttackFocus = true;
		}
	}

	// 通知主机与客户端播放表现动画（独立服务器自动跳过）
	MulticastPlayAttackMontage(SelectedIndex);

	// 服务端推进前摇命中与后摇结束定时器，携带 AttackId 防止串台
	FTimerManager& TimerManager = World->GetTimerManager();

	FTimerDelegate HitDelegate = FTimerDelegate::CreateUObject(
		this,
		&UEnemyCombatComponent::HandleAttackHitWindow,
		LaunchedAttackId
	);
	TimerManager.SetTimer(WindupTimerHandle, HitDelegate, WindupTime, false);

	FTimerDelegate FinishDelegate = FTimerDelegate::CreateUObject(
		this,
		&UEnemyCombatComponent::HandleAttackComplete,
		LaunchedAttackId
	);
	TimerManager.SetTimer(FinishTimerHandle, FinishDelegate, TotalDuration, false);

	return true;
}

void UEnemyCombatComponent::CancelAttack()
{
	AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	if (!IsValid(OwnerEnemy) || !OwnerEnemy->HasAuthority() || !bIsAttacking)
	{
		return;
	}

	ClearAttackTimers();
	ReleaseAttackFocus();

	bHitAttempted = true;
	bIsAttacking = false;
	ActiveAttackIndex = INDEX_NONE;
	LockedTarget.Reset();

	MulticastStopAttackMontage(0.15f);

	OnAttackFinished.Broadcast(true);
}

void UEnemyCombatComponent::HandleAttackHitWindow(uint32 InAttackId)
{
	AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	UWorld* World = GetWorld();
	if (!IsValid(OwnerEnemy) || !OwnerEnemy->HasAuthority() || !World)
	{
		return;
	}

	if (!bIsAttacking || InAttackId != CurrentAttackId || bHitAttempted)
	{
		return;
	}

	// 先标记已判定，再执行伤害逻辑，防止伤害回调（如目标死亡或反伤）重入
	bHitAttempted = true;

	// 到达命中时刻后解除瞄准焦点，让后摇保持挥击朝向
	ReleaseAttackFocus();

	if (OwnerEnemy->IsDead() || !IsValid(CombatConfig) || !CombatConfig->Attacks.IsValidIndex(ActiveAttackIndex))
	{
		return;
	}

	if (const UCharacterMovementComponent* Movement = OwnerEnemy->GetCharacterMovement())
	{
		if (Movement->IsFalling())
		{
			return;
		}
	}

	AActor* TargetActor = LockedTarget.Get();
	if (!IsValidAttackTarget(TargetActor))
	{
		UE_LOG(LogTPS_Shoot, Verbose, TEXT("[EnemyCombat] AttackId=%u 未命中：目标无效或已死亡"), InAttackId);
		return;
	}

	const FEnemyAttackSpec& Spec = CombatConfig->Attacks[ActiveAttackIndex];
	const FVector EnemyLocation = OwnerEnemy->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const FVector ToTarget = TargetLocation - EnemyLocation;

	// 1. 距离校验
	if (ToTarget.SizeSquared() > FMath::Square(Spec.AttackRange))
	{
		UE_LOG(LogTPS_Shoot, Verbose, TEXT("[EnemyCombat] AttackId=%u 未命中：超出攻击距离"), InAttackId);
		return;
	}

	// 2. 前方扇形夹角校验（2D 平面）
	const FVector HorizontalDirection = ToTarget.GetSafeNormal2D();
	const FVector Facing2D = OwnerEnemy->GetActorForwardVector().GetSafeNormal2D();
	const float MinCosAngle = FMath::Cos(FMath::DegreesToRadians(Spec.SwingHalfAngleDegrees));
	if (!HorizontalDirection.IsNearlyZero() && FVector::DotProduct(Facing2D, HorizontalDirection) < MinCosAngle)
	{
		UE_LOG(LogTPS_Shoot, Verbose, TEXT("[EnemyCombat] AttackId=%u 未命中：目标已脱离挥击扇形"), InAttackId);
		return;
	}

	// 3. 掩体/墙体遮挡校验
	UPrimitiveComponent* TargetRootPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	FVector ContactPoint = TargetLocation;
	if (IsValid(TargetRootPrimitive))
	{
		FVector ClosestPoint;
		if (TargetRootPrimitive->GetClosestPointOnCollision(EnemyLocation, ClosestPoint) >= 0.0f)
		{
			ContactPoint = ClosestPoint;
		}
	}

	const FVector TraceStart = EnemyLocation + FVector(0.0f, 0.0f, 35.0f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(EnemyMeleeObstruction), false);
	QueryParams.AddIgnoredActor(OwnerEnemy);
	QueryParams.AddIgnoredActor(TargetActor);

	FHitResult ObstructionHit;
	const bool bObstructed = World->LineTraceSingleByChannel(
		ObstructionHit,
		TraceStart,
		ContactPoint,
		CombatConfig->ObstructionTraceChannel,
		QueryParams
	);

#if !UE_BUILD_SHIPPING
	if (CombatConfig->bDrawDebugHitCheck)
	{
		const FColor TraceColor = bObstructed ? FColor::Red : FColor::Green;
		DrawDebugLine(World, TraceStart, ContactPoint, TraceColor, false, 1.0f, 0, 2.0f);
		DrawDebugSphere(World, ContactPoint, 12.0f, 8, TraceColor, false, 1.0f);
	}
#endif

	if (bObstructed)
	{
		UE_LOG(LogTPS_Shoot, Verbose, TEXT("[EnemyCombat] AttackId=%u 未命中：被 %s 遮挡"),
			InAttackId, *GetNameSafe(ObstructionHit.GetActor()));
		return;
	}

	// 命中成立：通过 UE 标准点伤害接口接入目标 UHealthComponent
	const FVector HitDirection = ToTarget.GetSafeNormal();
	FHitResult ContactHit(TargetActor, TargetRootPrimitive, ContactPoint, -HitDirection);

	UGameplayStatics::ApplyPointDamage(
		TargetActor,
		Spec.Damage,
		HitDirection,
		ContactHit,
		OwnerEnemy->GetController(),
		OwnerEnemy,
		nullptr
	);
}

void UEnemyCombatComponent::HandleAttackComplete(uint32 InAttackId)
{
	AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	UWorld* World = GetWorld();
	if (!IsValid(OwnerEnemy) || !OwnerEnemy->HasAuthority() || !World)
	{
		return;
	}

	if (!bIsAttacking || InAttackId != CurrentAttackId)
	{
		return;
	}

	float BaseCooldown = 1.0f;
	float CooldownVariation = 0.0f;
	if (IsValid(CombatConfig))
	{
		CooldownVariation = CombatConfig->CooldownVariation;
		if (CombatConfig->Attacks.IsValidIndex(ActiveAttackIndex))
		{
			BaseCooldown = CombatConfig->Attacks[ActiveAttackIndex].Cooldown;
		}
	}

	ClearAttackTimers();
	ReleaseAttackFocus();

	bIsAttacking = false;
	ActiveAttackIndex = INDEX_NONE;
	LockedTarget.Reset();

	// 叠加随机冷却偏差，避免多只敌人齐步挥击
	const float Jitter = CooldownVariation > 0.0f
		? FMath::FRandRange(-CooldownVariation, CooldownVariation)
		: 0.0f;
	NextAllowedAttackTime = World->GetTimeSeconds() + FMath::Max(0.1f, BaseCooldown + Jitter);

	OnAttackFinished.Broadcast(false);
}

bool UEnemyCombatComponent::IsValidAttackTarget(const AActor* TargetActor) const
{
	if (!IsValid(TargetActor) || !TargetActor->CanBeDamaged())
	{
		return false;
	}

	// 禁止攻击自己或友方敌人
	if (TargetActor == GetOwner() || TargetActor->IsA<AEnemyBase>())
	{
		return false;
	}

	if (const ATPS_ShootCharacter* Player = Cast<ATPS_ShootCharacter>(TargetActor))
	{
		return !Player->IsDead();
	}

	return true;
}

AEnemyBase* UEnemyCombatComponent::GetOwnerEnemy() const
{
	return Cast<AEnemyBase>(GetOwner());
}

AAIController* UEnemyCombatComponent::GetOwnerAIController() const
{
	if (const AEnemyBase* OwnerEnemy = GetOwnerEnemy())
	{
		return Cast<AAIController>(OwnerEnemy->GetController());
	}
	return nullptr;
}

void UEnemyCombatComponent::ClearAttackTimers()
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(WindupTimerHandle);
		TimerManager.ClearTimer(FinishTimerHandle);
	}
}

void UEnemyCombatComponent::ReleaseAttackFocus()
{
	if (!bHasAppliedAttackFocus)
	{
		return;
	}

	bHasAppliedAttackFocus = false;
	if (AAIController* AIController = GetOwnerAIController())
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void UEnemyCombatComponent::MulticastPlayAttackMontage_Implementation(int32 AttackIndex)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	if (!IsValid(OwnerEnemy) || OwnerEnemy->IsDead() || !IsValid(CombatConfig))
	{
		return;
	}

	if (!CombatConfig->Attacks.IsValidIndex(AttackIndex))
	{
		return;
	}

	const FEnemyAttackSpec& Spec = CombatConfig->Attacks[AttackIndex];
	if (!IsValid(Spec.AttackMontage))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = OwnerEnemy->GetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (IsValid(AnimInstance))
	{
		AnimInstance->Montage_Play(Spec.AttackMontage, Spec.PlayRate);
	}
}

void UEnemyCombatComponent::MulticastStopAttackMontage_Implementation(float BlendOutTime)
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	AEnemyBase* OwnerEnemy = GetOwnerEnemy();
	if (!IsValid(OwnerEnemy) || !IsValid(CombatConfig))
	{
		return;
	}

	USkeletalMeshComponent* Mesh = OwnerEnemy->GetMesh();
	UAnimInstance* AnimInstance = IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	if (!IsValid(AnimInstance))
	{
		return;
	}

	// 只停止正在播放的攻击蒙太奇，避免误杀其他表现动画
	for (const FEnemyAttackSpec& Spec : CombatConfig->Attacks)
	{
		if (IsValid(Spec.AttackMontage) && AnimInstance->Montage_IsPlaying(Spec.AttackMontage))
		{
			AnimInstance->Montage_Stop(BlendOutTime, Spec.AttackMontage);
		}
	}
}
