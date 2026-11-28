// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 实现约定：仅服务端监听伤害事件和执行频控；发送紧凑的事件数据，各端依据本地配置解析表现，独立服务器跳过视听播放。组件结束时解除旧数据源的委托。
 */

#include "Effect/HitFeedbackComponent.h"

#include "Effect/HitFeedbackConfig.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UHitFeedbackComponent::UHitFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHitFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority())
	{
		return;
	}

	if (UHealthComponent* HealthComp = OwnerActor->FindComponentByClass<UHealthComponent>())
	{
		ObservedHealthComponent = HealthComp;
		HealthComp->OnDamageTaken.AddUniqueDynamic(this, &UHitFeedbackComponent::HandleDamageTaken);
	}
}

void UHitFeedbackComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHealthComponent* HealthComp = ObservedHealthComponent.Get())
	{
		HealthComp->OnDamageTaken.RemoveDynamic(this, &UHitFeedbackComponent::HandleDamageTaken);
	}
	ObservedHealthComponent.Reset();

	Super::EndPlay(EndPlayReason);
}

void UHitFeedbackComponent::HandleDamageTaken(FDamageTakenEvent DamageEvent)
{
	const AActor* OwnerActor = GetOwner();
	const UWorld* World = GetWorld();
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority() || !World)
	{
		return;
	}

	if (!IsValid(FeedbackConfig) || DamageEvent.Damage <= 0.f)
	{
		return;
	}

	const double CurrentTime = World->GetTimeSeconds();
	const float MinInterval = FMath::Max(0.0f, FeedbackConfig->MinFeedbackInterval);

	// 高频集火/扫射节流：致命一击豁免节流，保证击杀反馈不丢失
	if (!DamageEvent.bIsFatal && MinInterval > 0.0f && LastFeedbackTime >= 0.0 && (CurrentTime - LastFeedbackTime) < MinInterval)
	{
		return;
	}
	LastFeedbackTime = CurrentTime;

	uint8 HitFlags = 0;
	if (DamageEvent.bIsHeadshot)
	{
		HitFlags |= HitFlag_Headshot;
	}
	if (DamageEvent.bIsFatal)
	{
		HitFlags |= HitFlag_Fatal;
	}

	const FVector SafeNormal = DamageEvent.HitNormal.IsNearlyZero()
		? FVector::UpVector
		: DamageEvent.HitNormal.GetSafeNormal();

	MulticastPlayHitFeedback(DamageEvent.HitLocation, SafeNormal, HitFlags);
}

void UHitFeedbackComponent::MulticastPlayHitFeedback_Implementation(
	FVector_NetQuantize HitLocation,
	FVector_NetQuantizeNormal HitNormal,
	uint8 HitFlags)
{
	if (GetNetMode() == NM_DedicatedServer || !IsValid(FeedbackConfig))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector SpawnLocation(HitLocation);
	const FVector NormalDir = FVector(HitNormal).GetSafeNormal();
	const FQuat BaseOrientation = NormalDir.IsNearlyZero()
		? FQuat::Identity
		: NormalDir.ToOrientationQuat();
	const FRotator SpawnRotation = (BaseOrientation * FeedbackConfig->EffectRotationOffset.Quaternion()).Rotator();

	if (UNiagaraSystem* VFX = ResolveHitVFX(HitFlags))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			VFX,
			SpawnLocation,
			SpawnRotation,
			FeedbackConfig->EffectScale,
			true,
			true,
			ENCPoolMethod::AutoRelease,
			true
		);
	}

	if (IsValid(FeedbackConfig->HitSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			FeedbackConfig->HitSound,
			SpawnLocation,
			FRotator::ZeroRotator,
			1.0f,
			1.0f,
			0.0f,
			FeedbackConfig->HitSoundAttenuation,
			FeedbackConfig->HitSoundConcurrency,
			GetOwner()
		);
	}
}

UNiagaraSystem* UHitFeedbackComponent::ResolveHitVFX(uint8 HitFlags) const
{
	if (!IsValid(FeedbackConfig))
	{
		return nullptr;
	}

	if ((HitFlags & HitFlag_Fatal) != 0 && IsValid(FeedbackConfig->FatalHitVFX))
	{
		return FeedbackConfig->FatalHitVFX;
	}

	if ((HitFlags & HitFlag_Headshot) != 0 && IsValid(FeedbackConfig->HeadshotHitVFX))
	{
		return FeedbackConfig->HeadshotHitVFX;
	}

	return FeedbackConfig->DefaultHitVFX;
}
