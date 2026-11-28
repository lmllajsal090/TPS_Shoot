// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：测试关卡中的简易可受伤目标，维护并复制自己的测试生命值。
 * 职责边界：用于独立验证，不承担真实角色的 HealthComponent 或敌人 AI 职责。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSTarget.generated.h"

UCLASS()
class TPS_SHOOT_API ATPSTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATPSTarget();

	UPROPERTY(EditDefaultsOnly, Category = "Target")
	float MaxHealth = 100.0f;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Target")
	float CurrentHealth = 0.0f;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount,const FDamageEvent& DamageEvent,AController* EventInstigator,AActor* DamageCauser) override;

};
