// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSTarget/TPSTarget.h"

#include "TPS_Shoot.h"

// Sets default values
ATPSTarget::ATPSTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATPSTarget::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth=MaxHealth;
}

float ATPSTarget::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	float Damage=Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth -= Damage;
	
	if (CurrentHealth<=0.f)
	{
		UE_LOG(LogTPS_Shoot,Warning,TEXT("Target Died"));
		Destroy();
		return Damage+CurrentHealth;
	}
	
	UE_LOG(LogTPS_Shoot,Warning,TEXT("Target Took %f Health"),CurrentHealth);
	return Damage;
}



