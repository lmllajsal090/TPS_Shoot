// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UTPSMainHUD.generated.h"

class UCrosshairWidget;
class UAmmoWidget;
/**
 * 
 */
UCLASS()
class TPS_SHOOT_API UUTPSMainHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UAmmoWidget>AmmoWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCrosshairWidget>Crosshair;
};
