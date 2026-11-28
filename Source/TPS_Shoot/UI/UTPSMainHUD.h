// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：主 HUD 容器，组织弹药、准星和属性条 Widget，并将当前玩家的武器与生命值等数据源连接到对应子 Widget。
 * 职责边界：负责初始化、观察和解除绑定，不修改武器或生命值状态，也不承担网络复制。
 */

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
