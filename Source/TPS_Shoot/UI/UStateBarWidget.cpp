// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UStateBarWidget.h"

#include "Components/ProgressBar.h"


void UUStateBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshDisplay();
}


void UUStateBarWidget::UpdateValue(float CurrentValue, float MaxValue)
{
	MaxHealth =FMath::Max(MaxValue, 0.f);

	CurrentHealth = FMath::Clamp(CurrentValue, 0.f, MaxHealth);

	RefreshDisplay();
}

float UUStateBarWidget::GetPercent() const
{
	if (MaxHealth <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentHealth / MaxHealth,0.f,1.f);
}

void UUStateBarWidget::RefreshDisplay()
{
	if (IsValid(HealthProgressBar))
	{
		HealthProgressBar->SetPercent(GetPercent());
	}
}