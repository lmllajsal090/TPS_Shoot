// Fill out your copyright notice in the Description page of Project Settings.

#include "Effect/HitFeedbackConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"

EDataValidationResult UHitFeedbackConfig::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);

	if (DefaultHitVFX == nullptr)
	{
		Context.AddWarning(FText::FromString(TEXT("UHitFeedbackConfig 未配置 DefaultHitVFX")));
	}

	if (HitSound == nullptr)
	{
		Context.AddWarning(FText::FromString(TEXT("UHitFeedbackConfig 未配置 HitSound")));
	}

	return SuperResult == EDataValidationResult::Invalid
		? EDataValidationResult::Invalid
		: EDataValidationResult::Valid;
}
#endif
