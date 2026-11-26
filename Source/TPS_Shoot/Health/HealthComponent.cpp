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

	OwnerActor->OnTakePointDamage.AddUniqueDynamic(
		this,
		&UHealthComponent::HandleTakePointDamage);

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

	// The local HUD may already be observing this component before BeginPlay.
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}


void UHealthComponent::ApplyDamage(float Damage)
{
	FDamageTakenEvent Event;
	Event.Damage = Damage;
	Event.bHasHitInfo = false;
	if (const AActor* OwnerActor = GetOwner())
	{
		Event.HitLocation = OwnerActor->GetActorLocation();
	}
	ApplyDamageInternal(Damage, Event);
}

void UHealthComponent::ApplyDamageInternal(float Damage, FDamageTakenEvent Event)
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

	Event.bIsFatal = bDiedThisTime;
	
	// 先更新完整状态，再通知监听者。
	bIsDead = bDiedThisTime;

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	OnDamageTaken.Broadcast(Event);

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
	const FPendingPointHit ConsumedHit = PendingPointHit;
	PendingPointHit.Reset();
	
	// 把具体扣血和广播封装进一个内部函数（现有 public 的 ApplyDamage(float) 也可以复用它）
	FDamageTakenEvent Event;
	Event.Damage = Damage;
	Event.InstigatedBy = InstigatedBy;
	Event.DamageCauser = DamageCauser;
	
	if (ConsumedHit.bValid)
	{
		Event.bHasHitInfo = true;
		Event.HitLocation = ConsumedHit.HitLocation;
		Event.HitNormal = ConsumedHit.HitNormal;
		Event.HitBoneName = ConsumedHit.BoneName;
	}
	else if (AActor* OwnerActor = GetOwner())
	{
		// 非点伤（如以后加的毒气/摔落）安全回退到角色中心
		Event.bHasHitInfo = false;
		Event.HitLocation = OwnerActor->GetActorLocation();
		Event.HitNormal = FVector::UpVector;
	}
	ApplyDamageInternal(Damage, Event);
	
}

void UHealthComponent::HandleTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
	FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection,
	const UDamageType* DamageType, AActor* DamageCauser)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bIsDead || Damage <= 0.f)
	{
		PendingPointHit.Reset();
		return;
	}
	PendingPointHit.bValid = true;
	PendingPointHit.HitLocation = HitLocation;
	// 你的子弹和敌人近战传入的 ShotFromDirection 都是“从攻击者指向目标”的方向，
	// 取反后正好指向外部（朝攻击者方向喷射血雾/火花）
	const FVector OutwardDir = -ShotFromDirection.GetSafeNormal();
	PendingPointHit.HitNormal = OutwardDir.IsNearlyZero() ? FVector::UpVector : OutwardDir;
	PendingPointHit.BoneName = BoneName;
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
