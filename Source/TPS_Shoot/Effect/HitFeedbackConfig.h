// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HitFeedbackConfig.generated.h"

class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;
class USoundConcurrency;

/**
 * 受击视听反馈数据资产（DataAsset）。
 * 将受击粒子、音效、朝向修正与高频节流参数与逻辑组件解耦，便于不同角色/敌人复用或差异化配置。
 */
UCLASS(BlueprintType)
class TPS_SHOOT_API UHitFeedbackConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 视觉特效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|VFX")
	TObjectPtr<UNiagaraSystem> DefaultHitVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|VFX")
	TObjectPtr<UNiagaraSystem> HeadshotHitVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|VFX")
	TObjectPtr<UNiagaraSystem> FatalHitVFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|VFX", meta = (ToolTip = "修正商店特效包默认喷射轴向差异"))
	FRotator EffectRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|VFX")
	FVector EffectScale = FVector::OneVector;

	// 声音效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|SFX")
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|SFX")
	TObjectPtr<USoundAttenuation> HitSoundAttenuation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|SFX")
	TObjectPtr<USoundConcurrency> HitSoundConcurrency;

	// 网络与频控
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Network", meta = (ToolTip = "高频集火/全自动扫射防刷屏最小触发间隔（秒），致命一击不受限制", ClampMin = "0.0", Units = "s"))
	float MinFeedbackInterval = 0.05f;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
