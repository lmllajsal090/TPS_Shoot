// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSWeapon/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "TPS_Shoot.h"
#include "NiagaraComponent.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	WeaponMesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	
	MuzzleFlashComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MuzzleFlashComponent"));
	MuzzleFlashComponent->SetupAttachment(WeaponMesh, WeaponMuzzleSocketName);
	
	MuzzleFlashComponent->SetAutoActivate(false);
}

void AWeaponBase::Fire(const FVector& AimPoint)
{
	if (bIsReloading)
	{
		return;
	}
	
	const double CurrentTime = GetWorld()->GetTimeSeconds();
	
	if (CurrentTime<NextAllowedFireTime)return;
	if (CurrentAmmo<=0)return;
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
	
	if (bHit&&IsValid(Hit.GetActor()))
	{
		const FVector ShootDirection = (AimPoint-MuzzleLocation).GetSafeNormal();
		
		UGameplayStatics::ApplyPointDamage(
			Hit.GetActor(),
			Damage,
			ShootDirection,
			Hit,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
			);
	}
	
	NextAllowedFireTime=CurrentTime+FireInterval;
	CurrentAmmo--;
	MuzzleFlashComponent->Activate(true);
	
	OnWeaponStateChanged.Broadcast();
	UE_LOG(LogTPS_Shoot, Log,
	TEXT("开火，剩余弹药：%d"), CurrentAmmo);
}

void AWeaponBase::StartReload()
{
	if (bIsReloading)return;
	if (CurrentAmmo>=MagazineCapacity)return;
	if (ReserveAmmo<=0)return;
	if (ReloadDuration<=0)return;
	
	bIsReloading=true;
	UE_LOG(LogTPS_Shoot, Log, TEXT("开始换弹"));
	
	GetWorldTimerManager().SetTimer(
	ReloadTimerHandle,
	this,
	&AWeaponBase::FinishReload,
	ReloadDuration,
	false
	);
	
	OnWeaponStateChanged.Broadcast();
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentAmmo = MagazineCapacity;
	
}

void AWeaponBase::FinishReload()
{
	if (!bIsReloading)
	{
		return;
	}
	const int32 MissingAmmo = MagazineCapacity - CurrentAmmo;
	const int32 AmmoToLoad = FMath::Min(MissingAmmo, ReserveAmmo);
	
	CurrentAmmo+=AmmoToLoad;
	ReserveAmmo-=AmmoToLoad;
	
	bIsReloading=false;
	
	OnWeaponStateChanged.Broadcast();
	UE_LOG(LogTPS_Shoot, Log, TEXT("换弹完成"));
	UE_LOG(LogTPS_Shoot, Log, TEXT("当前弹药：%d，备用弹药：%d"), CurrentAmmo, ReserveAmmo);
}


