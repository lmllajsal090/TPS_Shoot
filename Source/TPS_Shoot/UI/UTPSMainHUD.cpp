// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UTPSMainHUD.h"

#include "TPS_ShootCharacter.h"
#include "UI/AmmoWidget.h"

void UUTPSMainHUD::SetObservedCharacter(ATPS_ShootCharacter* Character)
{
	if (!IsValid(AmmoWidget))
	{
		return;
	}

	AWeaponBase* Weapon = nullptr;
	if (IsValid(Character))
	{
		Weapon = Character->EquippedWeapon.Get();
	}

	AmmoWidget->SetWeapon(Weapon);
}
