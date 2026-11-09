// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ATPSHUD.generated.h"

class UUTPSMainHUD;
/**
 * 
 */
UCLASS()
class TPS_SHOOT_API AATPSHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,Category="UI/HUD")
	TSubclassOf<UUTPSMainHUD>MainHUDClass;
	
	UPROPERTY()
	TObjectPtr<UUTPSMainHUD>MainHUD;
	
	virtual void BeginPlay() override;
};
