// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/CombatComponent.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
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
	if (CurrentWeapon==WeaponSlots[SlotIndex])
	{
		return;
	}
	
	if (!WeaponSlots[SlotIndex])return;
	
	CurrentWeapon = WeaponSlots[SlotIndex];
}

