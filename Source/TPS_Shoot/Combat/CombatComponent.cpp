// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatComponent.h"

#include "TPS_ShootCharacter.h"
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
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) ||!Character->HasAuthority() ||!IsValid(Character->EquippedWeapon) ||AimPoint.ContainsNaN())
	{
		return;
	}

	Character->EquippedWeapon->Fire(AimPoint);
}

void UCombatComponent::Reload()
{
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) ||
		!Character->HasAuthority() ||
		!IsValid(Character->EquippedWeapon))
	{
		return;
	}

	Character->EquippedWeapon->StartReload();
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
	
}


void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}

void UCombatComponent::SwitchWeapon(int32 SlotIndex)
{
	
	if (!IsValid(GetOwner()) || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (!WeaponSlots.IsValidIndex(SlotIndex))
	{
		return;
	}
	
	AWeaponBase* NewWeapon = WeaponSlots[SlotIndex];

	if (!IsValid(NewWeapon))
	{
		return;
	}
	
	ATPS_ShootCharacter* Character = Cast<ATPS_ShootCharacter>(GetOwner());

	if (!IsValid(Character) || Character->EquippedWeapon == NewWeapon)
	{
		return;
	}
	
	if (IsValid(Character->EquippedWeapon) &&Character->EquippedWeapon->IsReloading())
	{
		return;
	}
	
	// 隐藏当前武器。
	if (IsValid(Character->EquippedWeapon))
	{
		Character->EquippedWeapon->SetActorHiddenInGame(true);
	}

	// 显示目标武器，并更新角色当前装备。
	NewWeapon->SetActorHiddenInGame(false);
	Character->SetEquippedWeapon(NewWeapon);
}

