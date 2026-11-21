// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AEnemyAIController.generated.h"

class ATPS_ShootCharacter;
class UBehaviorTreeComponent;
class UBehaviorTree;
/**
 * 
 */
UCLASS()
class TPS_SHOOT_API AAEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AAEnemyAIController();
	
protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent>BehaviorTreeComponent;
	
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="AI|Tree")
	TObjectPtr<UBehaviorTree>BehaviorTree;
	
	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY()
	TObjectPtr<ATPS_ShootCharacter>TargetActor;
};
