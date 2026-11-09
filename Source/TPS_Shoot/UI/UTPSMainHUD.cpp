// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UTPSMainHUD.h"

#include "TPS_ShootCharacter.h"
#include "UStateBarWidget.h"
#include "Health/HealthComponent.h"
#include "UI/AmmoWidget.h"
#include "UI/CrosshairWidget.h"

void UUTPSMainHUD::SetObservedCharacter(ATPS_ShootCharacter* Character)
{
	if (ObservedHealthComponent.IsValid())
	{
		ObservedHealthComponent->OnHealthChanged.RemoveDynamic(this, &UUTPSMainHUD::HandleHealthChanged);
	}
	ObservedHealthComponent.Reset();
	
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
		
		ObservedHealthComponent=Character->FindComponentByClass<UHealthComponent>();
	}

	// Initialize immediately, even if the weapon replicated before we subscribed.
	HandleEquippedWeaponChanged(IsValid(Character) ? Character->EquippedWeapon.Get() : nullptr);

	if (IsValid(Crosshair))
	{
		Crosshair->SetObservedCharacter(Character);
	}
	
	if (ObservedHealthComponent.IsValid())
	{
		ObservedHealthComponent->OnHealthChanged.AddUniqueDynamic(this,&UUTPSMainHUD::HandleHealthChanged);
		float CurrentHealth=ObservedHealthComponent->GetCurrentHealth();
		float MaxHealth=ObservedHealthComponent->GetMaxHealth();
		HandleHealthChanged(CurrentHealth,MaxHealth);
	}else
	{
		HandleHealthChanged(0,0);
	}
}

void UUTPSMainHUD::HandleHealthChanged(float Current, float Max)
{
	if (StateBarWidget)
	{
		StateBarWidget->UpdateValue(Current,Max);
	}
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
