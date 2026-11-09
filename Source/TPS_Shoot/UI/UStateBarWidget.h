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
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|StatBar")
	TObjectPtr<UProgressBar> HealthProgressBar;
	
	virtual void NativeConstruct() override;

	// Blueprint controls presentation; the default implementation updates immediately.
	UFUNCTION(BlueprintNativeEvent, Category = "UI|StatBar")
	void OnPercentChanged(float TargetPercent, bool bImmediate);
	virtual void OnPercentChanged_Implementation(float TargetPercent, bool bImmediate);
public:
	UFUNCTION(BlueprintCallable, Category = "UI|StatBar")
	void UpdateValue(float CurrentValue, float MaxValue);

	UFUNCTION(BlueprintPure, Category = "UI|StatBar")
	float GetPercent() const;
private:
	void RefreshDisplay(bool bImmediate);
	
	float CurrentHealth = 0.f;
	float MaxHealth= 0.f;
	bool bHasValidValue = false;
};
