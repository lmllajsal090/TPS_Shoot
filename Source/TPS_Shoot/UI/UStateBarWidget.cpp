// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UStateBarWidget.h"

#include "Components/ProgressBar.h"


void UUStateBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshDisplay(true);
}


void UUStateBarWidget::UpdateValue(float CurrentValue, float MaxValue)
{
	MaxHealth =FMath::Max(MaxValue, 0.f);

	CurrentHealth = FMath::Clamp(CurrentValue, 0.f, MaxHealth);

	const bool bImmediate = !bHasValidValue || MaxHealth <= 0.f;
	bHasValidValue = MaxHealth > 0.f;
	RefreshDisplay(bImmediate);
}

float UUStateBarWidget::GetPercent() const
{
	if (MaxHealth <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(CurrentHealth / MaxHealth,0.f,1.f);
}

void UUStateBarWidget::RefreshDisplay(bool bImmediate)
{
	OnPercentChanged(GetPercent(), bImmediate);
}

void UUStateBarWidget::OnPercentChanged_Implementation(float TargetPercent, bool bImmediate)
{
	if (IsValid(HealthProgressBar))
	{
		HealthProgressBar->SetPercent(TargetPercent);
	}
}