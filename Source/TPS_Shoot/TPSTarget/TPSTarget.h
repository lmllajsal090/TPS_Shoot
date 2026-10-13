// Fill out your copyright notice in the Description page of Project Settings.

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

	UPROPERTY(VisibleInstanceOnly, Category = "Target")
	float CurrentHealth = 0.0f;
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount,const FDamageEvent& DamageEvent,AController* EventInstigator,AActor* DamageCauser) override;

};
