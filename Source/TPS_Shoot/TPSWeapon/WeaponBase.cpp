// Fill out your copyright notice in the Description page of Project Settings.


#include "TPSWeapon/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "TPS_Shoot.h"
#include "NiagaraComponent.h"
#include "TPS_ShootCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"
#include "Projectile/AProjectileBase.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
	
	bReplicates = true;
	AActor::SetReplicateMovement(true);
	
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
	if (bIsReloading||!HasAuthority())
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
	MulticastPlayFireEffects();
	
	OnWeaponStateChanged.Broadcast();
}

void AWeaponBase::StartReload()
{
	if (!HasAuthority() || bIsReloading||CurrentAmmo>=MagazineCapacity||ReserveAmmo<=0||ReloadDuration<=0)return;
	
	bIsReloading=true;
	
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
	if (HasAuthority())
	{
		CurrentAmmo = MagazineCapacity;
	}
	
}

void AWeaponBase::OnRep_WeaponState()
{

	OnWeaponStateChanged.Broadcast();
}

void AWeaponBase::MulticastPlayFireEffects_Implementation()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (IsValid(MuzzleFlashComponent))
	{
		MuzzleFlashComponent->Activate(true);
	}

	if (IsValid(FireSound))
	{
		FVector SoundLocation = GetActorLocation();

		if (IsValid(WeaponMesh) &&WeaponMesh->DoesSocketExist(WeaponMuzzleSocketName))
		{
			SoundLocation =WeaponMesh->GetSocketLocation(WeaponMuzzleSocketName);
		}

		UGameplayStatics::PlaySoundAtLocation(
			this,
			FireSound,
			SoundLocation,
			FRotator::ZeroRotator,
			1.f,                 // 音量倍率
			1.f,                 // 音高倍率
			0.f,                 // 从声音开头播放
			FireAttenuation,
			FireConcurrency,
			this                 // 并发规则使用的声音拥有者
			);
	}
	
	if (ATPS_ShootCharacter* Character =Cast<ATPS_ShootCharacter>(GetOwner()))
	{
		Character->ApplyRecoil(RecoilPitch, RecoilYawRange);
		Character->NotifyCrosshairShot(CrosshairSpreadPerShot, CrosshairMaxSpread, CrosshairRecoverySpeed);
	}
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ReloadTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AWeaponBase::FinishReload()
{
	if (!bIsReloading||!HasAuthority())
	{
		return;
	}
	const int32 MissingAmmo = MagazineCapacity - CurrentAmmo;
	const int32 AmmoToLoad = FMath::Min(MissingAmmo, ReserveAmmo);
	
	CurrentAmmo+=AmmoToLoad;
	ReserveAmmo-=AmmoToLoad;
	
	bIsReloading=false;
	
	OnWeaponStateChanged.Broadcast();
}

FTransform AWeaponBase::GetLeftHandGripTransform_Implementation() const
{
	return GetActorTransform();
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeaponBase, CurrentAmmo);
	DOREPLIFETIME(AWeaponBase, ReserveAmmo);
	DOREPLIFETIME(AWeaponBase, bIsReloading);
}



