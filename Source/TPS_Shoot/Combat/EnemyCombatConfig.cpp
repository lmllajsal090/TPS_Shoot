// Fill out your copyright notice in the Description page of Project Settings.

#include "Combat/EnemyCombatConfig.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

float UEnemyCombatConfig::GetMaxAttackRange() const
{
	float MaxRange = 0.0f;
	for (const FEnemyAttackSpec& Spec : Attacks)
	{
		MaxRange = FMath::Max(MaxRange, Spec.AttackRange);
	}
	return MaxRange;
}

int32 UEnemyCombatConfig::SelectAttackIndex(float DistanceToTarget, int32 LastAttackIndex) const
{
	if (Attacks.IsEmpty())
	{
		return INDEX_NONE;
	}

	TArray<int32> EligibleIndices;
	EligibleIndices.Reserve(Attacks.Num());

	// 把所有可用的招式的索引存储起来
	for (int32 Index = 0; Index < Attacks.Num(); ++Index)
	{
		const FEnemyAttackSpec& Spec = Attacks[Index];
		if (DistanceToTarget <= Spec.AttackRange && Spec.SelectionWeight > 0.0f)
		{
			EligibleIndices.Add(Index);
		}
	}

	if (EligibleIndices.IsEmpty())
	{
		return INDEX_NONE;
	}

	// 若有多个可用招式，尽量避免连续出同一招。
	if (EligibleIndices.Num() > 1 && LastAttackIndex != INDEX_NONE)
	{
		EligibleIndices.Remove(LastAttackIndex);
	}

	float TotalWeight = 0.0f;
	for (const int32 Index : EligibleIndices)
	{
		TotalWeight += Attacks[Index].SelectionWeight;
	}

	if (TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return EligibleIndices[0];
	}

	float Roll = FMath::FRandRange(0.0f, TotalWeight);
	for (const int32 Index : EligibleIndices)
	{
		Roll -= Attacks[Index].SelectionWeight;
		if (Roll <= 0.0f)
		{
			return Index;
		}
	}

	return EligibleIndices.Last();
}

#if WITH_EDITOR
EDataValidationResult UEnemyCombatConfig::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Attacks.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("EnemyCombatConfig 至少需要配置一个攻击招式（Attacks 不能为空）。")));
		return EDataValidationResult::Invalid;
	}

	for (int32 Index = 0; Index < Attacks.Num(); ++Index)
	{
		const FEnemyAttackSpec& Spec = Attacks[Index];
		if (!IsValid(Spec.AttackMontage))
		{
			Context.AddWarning(FText::FromString(
				FString::Printf(TEXT("Attacks[%d] 未配置 AttackMontage，客户端将没有攻击动作表现。"), Index)));
		}

		if (Spec.WindupTime <= 0.0f || Spec.RecoveryTime <= 0.0f)
		{
			Context.AddError(FText::FromString(
				FString::Printf(TEXT("Attacks[%d] 的 WindupTime 与 RecoveryTime 必须大于 0。"), Index)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result == EDataValidationResult::NotValidated ? EDataValidationResult::Valid : Result;
}
#endif
