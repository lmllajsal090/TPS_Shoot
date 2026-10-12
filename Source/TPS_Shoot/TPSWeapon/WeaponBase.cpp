// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSWeapon/WeaponBase.h"

#include "TPS_Shoot.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	WeaponMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
}

void AWeaponBase::Fire(const FVector& AimPoint)
{
	if (WeaponMuzzleSocketName.IsNone() ||
	!WeaponMesh->DoesSocketExist(WeaponMuzzleSocketName))
	{
		UE_LOG(LogTPS_Shoot, Warning, TEXT("枪口 Socket 未设置或不存在"));
		return;
	}
	FVector MuzzleLocation = WeaponMesh->GetSocketLocation(WeaponMuzzleSocketName);
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());
	
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		MuzzleLocation,
		AimPoint,
		ECollisionChannel::ECC_Visibility,
		Params
	);
	
	DrawDebugLine(
		GetWorld(),
		MuzzleLocation,
		bHit?Hit.ImpactPoint:AimPoint,
		FColor::Red,
		false,
		1.0f,
		0,
		2.0f
		);
	
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}


