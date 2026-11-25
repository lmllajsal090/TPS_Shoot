// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UHitFeedbackConfig.generated.h"

class UNiagaraSystem;
class USoundBase;
class USoundAttenuation;
class USoundConcurrency;
/**
 * 
 */
UCLASS()
class TPS_SHOOT_API UUHitFeedbackConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	//视觉特效
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|VFX")
	TObjectPtr<UNiagaraSystem> DefaultHitVFX;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|VFX")
	TObjectPtr<UNiagaraSystem> HeadshotHitVFX;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|VFX",meta=(ToolTip="修正商店特效包轴向差异"))
	FRotator EffectRotationOffset = FRotator::ZeroRotator;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|VFX")
	FVector EffectScale = FVector::OneVector;
	
	//声音效果
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|SFX")
	TObjectPtr<USoundBase> HitSound;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|SFX")
	TObjectPtr<USoundAttenuation> HitSoundAttenuation;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect|SFX")
	TObjectPtr<USoundConcurrency> HitSoundConcurrency;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Effect",meta=(ToolTip="高频集火/霰弹防刷屏节流间隔",ClampMin="0.0"))
	float MinFeedbackInterval = 0.05f;
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
