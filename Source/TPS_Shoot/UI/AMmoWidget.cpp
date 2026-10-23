// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoWidget.h"
#include "Components/TextBlock.h"
#include "TPSWeapon/WeaponBase.h"

void UAmmoWidget::SetWeapon(AWeaponBase* InWeapon)
{
	if (IsValid(ObservedWeapon)&&StateChangedHandle.IsValid())
	{
		ObservedWeapon->OnWeaponStateChanged.Remove(StateChangedHandle);
	}
	StateChangedHandle.Reset();
	ObservedWeapon = InWeapon;
	
	if (IsValid(ObservedWeapon))
	{
		StateChangedHandle = ObservedWeapon->OnWeaponStateChanged.AddUObject(this, &UAmmoWidget::RefreshDisplay);
	}
	
	RefreshDisplay();
}

void UAmmoWidget::NativeDestruct()
{
	if (IsValid(ObservedWeapon)&&StateChangedHandle.IsValid())
	{
		ObservedWeapon->OnWeaponStateChanged.Remove(StateChangedHandle);
	}
	
	StateChangedHandle.Reset();
	
	Super::NativeDestruct();
}

void UAmmoWidget::RefreshDisplay()
{
	if (!IsValid(ObservedWeapon))
	{
		AmmoText->SetText(FText::FromString(TEXT("0 / 0")));
		ReserveText->SetText(FText::FromString(TEXT("备用：0")));
		ReloadText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	
	AmmoText->SetText(FText::FromString(FString::Printf(
		TEXT("%d / %d"),
		ObservedWeapon->GetCurrentAmmo(),
		ObservedWeapon->GetMagazineCapacity())));

	ReserveText->SetText(FText::FromString(FString::Printf(
		TEXT("备用：%d"),
		ObservedWeapon->GetReserveAmmo())));

	ReloadText->SetText(FText::FromString(TEXT("换弹中")));

	ReloadText->SetVisibility(
		ObservedWeapon->IsReloading()
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
}
