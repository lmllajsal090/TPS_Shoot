// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSTarget/TPSTarget.h"

#include "Net/UnrealNetwork.h"

// Sets default values
ATPSTarget::ATPSTarget()
{
	bReplicates = true;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATPSTarget::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATPSTarget, CurrentHealth);
}

// Called when the game starts or when spawned
void ATPSTarget::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CurrentHealth=MaxHealth;
	}
}

float ATPSTarget::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (!HasAuthority()|| CurrentHealth <= 0.f|| !FMath::IsFinite(DamageAmount)|| DamageAmount <= 0.f)
	{
		return 0.f;
	}
	
	float Damage=Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= Damage;
	
	if (CurrentHealth<=0.f)
	{
		Destroy();
		return Damage;
	}
	
	return Damage;
}



