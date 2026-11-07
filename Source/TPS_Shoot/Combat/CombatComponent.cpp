// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatComponent.h"
#include "Engine/World.h"
#include "TPS_ShootCharacter.h"
#include "Animation/AnimMontage.h"
#include "Net/UnrealNetwork.h"
#include "TPSWeapon/WeaponBase.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


void UCombatComponent::RegisterInitialWeapon(AWeaponBase* Weapon)
{
	if (!IsValid(GetOwner()) ||!GetOwner()->HasAuthority() ||!IsValid(Weapon))
	{
		return;
	}

	WeaponSlots.AddUnique(Weapon);
}

void UCombatComponent::Fire(const FVector& AimPoint)
{
	if (bIsSwitchingWeapon)
	{
		return;
	}
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) || !Character->HasAuthority() ||
		Character->IsDead() || AimPoint.ContainsNaN())
	{
		return;
	}

	AWeaponBase* CurrentWeapon = GetCurrentWeapon();
	if (!IsValid(CurrentWeapon))
	{
		return;
	}

	CurrentWeapon->Fire(AimPoint);
}

void UCombatComponent::Reload()
{
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) ||!Character->HasAuthority() ||Character->IsDead() ||
		!IsValid(Character->EquippedWeapon)||bIsSwitchingWeapon)
	{
		return;
	}

	GetCurrentWeapon()->StartReload();
}

void UCombatComponent::StartFire()
{
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());
	if (!IsValid(Character) || !Character->IsLocallyControlled() ||
		Character->IsDead() || bIsSwitchingWeapon)
	{
		return;
	}

	AWeaponBase* CurrentWeapon = GetCurrentWeapon();
	if (!IsValid(CurrentWeapon)||bFireButtonPressed)
	{
		return;
	}
	
	FiringWeapon=CurrentWeapon;
	bFireButtonPressed=true;
	TryFire();
	
}

void UCombatComponent::ServerFire_Implementation(FVector_NetQuantize AimPoint)
{
	Fire(AimPoint);
}

void UCombatComponent::StopFire()
{
	bFireButtonPressed = false;
	FiringWeapon .Reset();

	// 清除定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FireTimerHandle);
	}
}

void UCombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GetOwner()) && GetOwner()->HasAuthority())
	{
		for (AWeaponBase* Weapon:WeaponSlots)
		{
			if (IsValid(Weapon))
			{
				Weapon->Destroy();
			}
		}
		WeaponSlots.Empty(); 
	}
	StopFire();
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(WeaponSwitchCommitTimerHandle);
		TimerManager.ClearTimer(WeaponSwitchFinishTimerHandle);
		TimerManager.ClearTimer(WeaponSwitchHideTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCombatComponent::AttachWeaponToSocket(AWeaponBase* Weapon, FName SocketName)
{
	
}

void UCombatComponent::FinishWeaponSwitch()
{
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();

		TimerManager.ClearTimer(WeaponSwitchCommitTimerHandle);
		TimerManager.ClearTimer(WeaponSwitchFinishTimerHandle);
		TimerManager.ClearTimer(WeaponSwitchHideTimerHandle);
	}

	PendingWeapon = nullptr;
	bIsSwitchingWeapon = false;
}

void UCombatComponent::CancelWeaponSwitch()
{
	// Equipment and weapon visibility remain server-authoritative.
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		return;
	}

	// Clears all three timers, PendingWeapon, and the switching flag.
	FinishWeaponSwitch();

	// Keep whichever weapon was equipped when the switch was interrupted.
	AWeaponBase* CurrentWeapon = GetCurrentWeapon();
	for (AWeaponBase* Weapon : WeaponSlots)
	{
		if (IsValid(Weapon))
		{
			Weapon->SetActorHiddenInGame(Weapon != CurrentWeapon);
		}
	}
}

void UCombatComponent::TryFire()
{
	ATPS_ShootCharacter* Character =Cast<ATPS_ShootCharacter>(GetOwner());

	AWeaponBase* CurrentWeapon = GetCurrentWeapon();

	if (!IsValid(Character) ||
		Character->IsDead() ||
		!Character->IsLocallyControlled() ||
		!bFireButtonPressed ||
		!IsValid(CurrentWeapon) ||
		FiringWeapon.Get() != CurrentWeapon)
	{
		StopFire();
		return;
	}

	if (CurrentWeapon->IsReloading() ||CurrentWeapon->GetCurrentAmmo() <= 0)
	{
		StopFire();
		return;
	}

	const FVector AimPoint = Character->GetAimPoint();

	if (AimPoint.ContainsNaN())
	{
		StopFire();
		return;
	}

	// 每次请求都重新计算瞄准点，是否实际射击由服务器决定。
	ServerFire(AimPoint);

	// 半自动只请求这一发，等待松开后再次按下。
	if (CurrentWeapon->GetFireMode() != EFireMode::Automatic)
	{
		return;
	}

	const float Interval = CurrentWeapon->GetFireInterval();
	UWorld* World = GetWorld();

	if (!World || !FMath::IsFinite(Interval) || Interval <= 0.f)
	{
		StopFire();
		return;
	}

	// 等待一个射击间隔，再次进入 TryFire 检查并请求下一发。
	World->GetTimerManager().SetTimer(
		FireTimerHandle,
		this,
		&UCombatComponent::TryFire,
		Interval,
		false
	);
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, bIsSwitchingWeapon);
}

void UCombatComponent::CommitWeaponSwitch()
{
	ATPS_ShootCharacter* Character =Cast<ATPS_ShootCharacter>(GetOwner());
	// 实际装备变更只由服务器执行。
	if (!IsValid(Character) || !Character->HasAuthority())
	{
		return;
	}

	if (Character->IsDead())
	{
		CancelWeaponSwitch();
		return;
	}

	if (!bIsSwitchingWeapon)
	{
		return;
	}
	
	if (!IsValid(PendingWeapon))
	{
		FinishWeaponSwitch();
		return;
	}
	PendingWeapon->SetActorHiddenInGame(false);
	Character->SetEquippedWeapon(PendingWeapon.Get());
}

void UCombatComponent::HideCurrentWeapon()
{
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());
	if (!IsValid(Character) || !Character->HasAuthority())
	{
		return;
	}

	if (Character->IsDead())
	{
		CancelWeaponSwitch();
		return;
	}

	AWeaponBase* CurrentWeapon = GetCurrentWeapon();
	
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->SetActorHiddenInGame(true);
	}
	
}

void UCombatComponent::SwitchWeapon(int32 SlotIndex)
{
	ATPS_ShootCharacter* Character =Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) || !Character->HasAuthority() || Character->IsDead())
	{
		return;
	}

	// 正在切枪时，不接受新的切枪请求。
	if (bIsSwitchingWeapon)
	{
		return;
	}

	if (!WeaponSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	AWeaponBase* NewWeapon = WeaponSlots[SlotIndex];
	AWeaponBase* CurrentWeapon = GetCurrentWeapon();

	if (!IsValid(NewWeapon) || NewWeapon == CurrentWeapon)
	{
		return;
	}

	// 第一版不允许在换弹过程中切枪。
	if (IsValid(CurrentWeapon) && CurrentWeapon->IsReloading())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UAnimMontage* SwitchMontage = nullptr;
	float CommitTime = 0.f;
	float HideTime=0.f;

	// 根据目标武器选择动作及实际换枪时刻。
	switch (NewWeapon->GetWeaponType())
	{
	case EWeaponType::Pistol:
		SwitchMontage = AM_SwitchToPistol.Get();
		HideTime = 0.4f;
		CommitTime=0.82f;
		break;

	case EWeaponType::Rifle:
		SwitchMontage = AM_SwitchToRifle.Get();
		HideTime = 0.26f;
		CommitTime=0.56f;
		break;

	default:
		return;
	}

	if (!IsValid(SwitchMontage))
	{
		return;
	}

	// 目前约定 Montage 以 1 倍速播放。
	const float FinishTime = SwitchMontage->GetPlayLength();

	StopFire();

	PendingWeapon = NewWeapon;
	bIsSwitchingWeapon = true;

	// 此处由服务器调用，通知服务器和相关客户端播放。
	MulticastPlayWeaponSwitch(SwitchMontage);
	
	FTimerManager& TimerManager = World->GetTimerManager();

	// 动作中途，真正更换装备。
	TimerManager.SetTimer(
		WeaponSwitchCommitTimerHandle,
		this,
		&UCombatComponent::CommitWeaponSwitch,
		CommitTime,
		false
	);

	TimerManager.SetTimer(
		WeaponSwitchHideTimerHandle,
		this,
		&UCombatComponent::HideCurrentWeapon,
		HideTime,
		false
	);
	
	// 整个动作时间结束，解除切枪状态。
	TimerManager.SetTimer(
		WeaponSwitchFinishTimerHandle,
		this,
		&UCombatComponent::FinishWeaponSwitch,
		FinishTime,
		false
	);
}

AWeaponBase* UCombatComponent::GetCurrentWeapon() const
{
	ATPS_ShootCharacter* Character=Cast<ATPS_ShootCharacter>(GetOwner());
	if (IsValid(Character))
	{
		return Character->EquippedWeapon;
	}
	return nullptr;
}

void UCombatComponent::MulticastPlayWeaponSwitch_Implementation(UAnimMontage* SwitchMontage)
{
	StopFire();
	// 独立服务器不需要播放表现动画。
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	
	ATPS_ShootCharacter* Character =Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) || Character->IsDead() || !IsValid(SwitchMontage))
	{
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	UAnimInstance* AnimInstance = IsValid(CharacterMesh) ? CharacterMesh->GetAnimInstance() : nullptr;
	if (IsValid(AnimInstance))
	{
		AnimInstance->Montage_Play(SwitchMontage, 1.f);
	}
}


