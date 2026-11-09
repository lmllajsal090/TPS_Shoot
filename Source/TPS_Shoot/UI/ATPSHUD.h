// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ATPSHUD.generated.h"

class APawn;
class UUTPSMainHUD;

UCLASS()
class TPS_SHOOT_API AATPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "UI/HUD")
	TSubclassOf<UUTPSMainHUD> MainHUDClass;

	UPROPERTY(Transient)
	TObjectPtr<UUTPSMainHUD> MainHUD;

	virtual void BeginPlay() override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
};
