// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：敌人角色的公共宿主，装配生命、战斗和受击反馈组件，并统一角色死亡后的清理与表现入口。
 * 职责边界：AI 决策交给 AIController；战斗动作执行及生命值结算交给相应组件；死亡动画由蓝图负责。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UEnemyCombatComponent;
class UHealthComponent;
class UHitFeedbackComponent;

UCLASS()
class TPS_SHOOT_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UEnemyCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHitFeedbackComponent> HitFeedbackComponent;

	UFUNCTION()
	void HandleDeath();

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

	bool bDeathHandled = false;

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Death")
	void OnDeathPresentation();

public:
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UEnemyCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "HitFeedback")
	UHitFeedbackComponent* GetHitFeedbackComponent() const { return HitFeedbackComponent; }
};
