// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UTPSMainHUD.h"

#include "TPS_ShootCharacter.h"
#include "UI/AmmoWidget.h"

void UUTPSMainHUD::SetObservedCharacter(ATPS_ShootCharacter* Character)
{
	if (ObservedCharacter.IsValid() && EquippedWeaponChangedHandle.IsValid())
	{
		ObservedCharacter->OnEquippedWeaponChanged.Remove(EquippedWeaponChangedHandle);
	}
	EquippedWeaponChangedHandle.Reset();
	ObservedCharacter = Character;

	if (IsValid(Character))
	{
		EquippedWeaponChangedHandle = Character->OnEquippedWeaponChanged.AddUObject(
			this, &UUTPSMainHUD::HandleEquippedWeaponChanged);
	}

	// Initialize immediately, even if the weapon replicated before we subscribed.
	HandleEquippedWeaponChanged(IsValid(Character) ? Character->EquippedWeapon.Get() : nullptr);
}

void UUTPSMainHUD::HandleEquippedWeaponChanged(AWeaponBase* Weapon)
{
	if (IsValid(AmmoWidget))
	{
		AmmoWidget->SetWeapon(Weapon);
	}
}

void UUTPSMainHUD::NativeDestruct()
{
	SetObservedCharacter(nullptr);
	Super::NativeDestruct();
}
