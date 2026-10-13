// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoWidget.generated.h"

class AWeaponBase;
class UTextBlock;

UCLASS()
class TPS_SHOOT_API UAmmoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWeapon(AWeaponBase* InWeapon);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReserveText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReloadText;

private:
	void RefreshDisplay();

	UPROPERTY(Transient)
	TObjectPtr<AWeaponBase> ObservedWeapon;

	FDelegateHandle StateChangedHandle;
};