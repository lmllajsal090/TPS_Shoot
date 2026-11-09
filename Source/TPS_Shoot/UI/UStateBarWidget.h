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
	
	virtual void NativeConstruct() override;
public:
	UFUNCTION(BlueprintCallable, Category = "UI|StatBar")
	void UpdateValue(float CurrentValue, float MaxValue);

	UFUNCTION(BlueprintPure, Category = "UI|StatBar")
	float GetPercent() const;
private:
	void RefreshDisplay();
	
	float CurrentHealth = 0.f;
	float MaxHealth= 0.f;
};
