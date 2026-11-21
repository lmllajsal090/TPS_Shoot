// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Health/HealthComponent.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	HealthComponent =CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this,&AEnemyBase::HandleDeath);
		if (IsDead())
		{
			HandleDeath();
		}
	}
}

void AEnemyBase::HandleDeath()
{
	if (bDeathHandled)
	{
		return;
	}

	bDeathHandled = true;
	
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


