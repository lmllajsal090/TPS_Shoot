// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UStateBarWidget.h"

#include "Components/ProgressBar.h"

void UUStateBarWidget::UpdateValue(float CurrentValue, float MaxValue)
{
	if (!HealthProgressBar)return;
	
	const float Percent=
		MaxValue>0.f
	?FMath::Clamp(CurrentValue/MaxValue,0.f,1.f):0.f;
	
	HealthProgressBar->SetPercent(Percent);
}
