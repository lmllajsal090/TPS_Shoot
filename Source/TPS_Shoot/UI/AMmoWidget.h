// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：观察当前武器的状态变化，展示弹药、备弹和换弹提示；切换数据源时解除旧委托。
 * 职责边界：只读展示武器状态，不修改弹药或发起武器操作。
 */

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