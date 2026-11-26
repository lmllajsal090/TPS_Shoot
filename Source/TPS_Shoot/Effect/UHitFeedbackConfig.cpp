// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/UHitFeedbackConfig.h"

#include "Misc/DataValidation.h"

EDataValidationResult UUHitFeedbackConfig::IsDataValid(class FDataValidationContext& Context) const
{
	return Super::IsDataValid(Context);
	
	if (DefaultHitVFX==nullptr)
	{
		Context.AddWarning(FText::FromString(TEXT("未配置特效")));
	}
	
	if (HitSound==nullptr)
	{
		Context.AddWarning(FText::FromString(TEXT("未配置声音")));
	}
}
