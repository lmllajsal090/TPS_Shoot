// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "UStateBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class TPS_SHOOT_API UUStateBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

public:
	UFUNCTION(BlueprintCallable)
	void UpdateValue(float CurrentValue, float MaxValue);
	
private:
	float MaxHealth;
	float CurrentHealth;
};
