// Fill out your copyright notice in the Description page of Project Settings.


#include "Health/HealthComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	AActor* OwnerActor = GetOwner();
	
	if (!OwnerActor || !OwnerActor->HasAuthority())
	{
		return;
	}

	InitializeHealth();

	OwnerActor->OnTakeAnyDamage.AddUniqueDynamic(
		this,
		&UHealthComponent::HandleTakeAnyDamage);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent,CurrentHealth);
	DOREPLIFETIME(UHealthComponent,MaxHealth);
	DOREPLIFETIME(UHealthComponent,bIsDead);
}

void UHealthComponent::InitializeHealth()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	MaxHealth = FMath::Max(MaxHealth, 1.f);
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

void UHealthComponent::ApplyDamage(float Damage)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() ||
		bIsDead || !FMath::IsFinite(Damage) || Damage <= 0.f)
	{
		return;
	}

	const float OldHealth = CurrentHealth;

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);

	if (CurrentHealth == OldHealth)
	{
		return;
	}

	const bool bDiedThisTime = CurrentHealth <= 0.f;

	// 先更新完整状态，再通知监听者。
	bIsDead = bDiedThisTime;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (bDiedThisTime)
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::Heal(float HealValue)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() ||
		bIsDead || !FMath::IsFinite(HealValue) || HealValue <= 0.f)
	{
		return;
	}

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + HealValue, 0.f, MaxHealth);

	if (CurrentHealth != OldHealth)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	ApplyDamage(Damage);
}

void UHealthComponent::OnRep_HealthValues()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		OnDeath.Broadcast();
	}
}
