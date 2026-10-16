// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSWeapon/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "TPS_Shoot.h"
#include "NiagaraComponent.h"
#include "Projectile/AProjectileBase.h"

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
	
	if (!ProjectileClass)
	{
		UE_LOG(LogTPS_Shoot, Warning, TEXT("未设置弹丸类"));
		return;
	}
	
	const FVector ShootDirection = (AimPoint-MuzzleLocation).GetSafeNormal();
	
	if (ShootDirection.IsNearlyZero())
	{
		return;
	}
	const FRotator SpawnRotation = ShootDirection.Rotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride =ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
	
	AAProjectileBase* Projectile =GetWorld()->SpawnActor<AAProjectileBase>(
		ProjectileClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParams
	);
	
	if (!IsValid(Projectile))
	{
		UE_LOG(LogTPS_Shoot, Warning, TEXT("子弹生成失败"));
		return;
	}
	
		Projectile->SetDamage(Damage);
	
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


