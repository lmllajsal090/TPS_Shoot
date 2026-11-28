// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 实现约定：死亡通知可能由服务器本地状态或客户端复制结果触发，角色通过本地标记防止重复处理；权威端停止 AI 和战斗，各端负责自身移动、碰撞与蓝图死亡表现。
 */


#include "EnemyBase.h"

#include "AI/AEnemyAIController.h"
#include "Combat/EnemyCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Effect/HitFeedbackComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Health/HealthComponent.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("CombatComponent"));
	HitFeedbackComponent = CreateDefaultSubobject<UHitFeedbackComponent>(TEXT("HitFeedbackComponent"));
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddUniqueDynamic(this, &AEnemyBase::HandleTakeAnyDamage);
	}
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &AEnemyBase::HandleDeath);
		if (IsDead())
		{
			HandleDeath();
		}
	}
}

void AEnemyBase::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (!HasAuthority() || IsDead() || Damage <= 0.0f)
	{
		return;
	}

	if (AAEnemyAIController* AIController = Cast<AAEnemyAIController>(GetController()))
	{
		AIController->NotifyDamagedBy(InstigatedBy);
	}
}

void AEnemyBase::HandleDeath()
{
	if (bDeathHandled)
	{
		return;
	}

	bDeathHandled = true;

	if (HasAuthority())
	{
		if (IsValid(CombatComponent))
		{
			CombatComponent->CancelAttack();
		}

		if (AAEnemyAIController* AIController = Cast<AAEnemyAIController>(GetController()))
		{
			AIController->StopAIOnDeath();
		}
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.1f);
	}

	OnDeathPresentation();
}

bool AEnemyBase::IsDead() const
{
	return IsValid(HealthComponent)&&HealthComponent->GetIsDead();
}


