// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

class UHealthComponent;
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

	UFUNCTION()
	void HandleDeath();
	
	bool bDeathHandled=false;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Death")
	void OnDeathPresentation();
public:	
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;
};
