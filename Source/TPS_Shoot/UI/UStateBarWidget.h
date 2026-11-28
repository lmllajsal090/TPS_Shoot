// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：通用属性条显示组件，保存当前值/最大值、计算百分比，并将目标百分比交给蓝图表现。
 * 职责边界：不绑定 HealthComponent，不持有角色业务状态，也不负责网络复制。
 */

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
