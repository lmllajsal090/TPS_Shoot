// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UTPSMainHUD.generated.h"

class UHealthComponent;
class UUStateBarWidget;
class ATPS_ShootCharacter;
class AWeaponBase;
class UCrosshairWidget;
class UAmmoWidget;

UCLASS()
class TPS_SHOOT_API UUTPSMainHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAmmoWidget> AmmoWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCrosshairWidget> Crosshair;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUStateBarWidget> StateBarWidget;

	UFUNCTION()
	void SetObservedCharacter(ATPS_ShootCharacter* Character);
	
	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);
protected:
	virtual void NativeDestruct() override;

private:
	void HandleEquippedWeaponChanged(AWeaponBase* Weapon);

	UPROPERTY(Transient)
	TWeakObjectPtr<ATPS_ShootCharacter> ObservedCharacter;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<UHealthComponent> ObservedHealthComponent;
	
	FDelegateHandle EquippedWeaponChangedHandle;
};
