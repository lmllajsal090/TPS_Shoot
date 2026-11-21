// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AEnemyAIController.generated.h"

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
	virtual void OnUnPossess() override;

private:
	UFUNCTION()
	void HandleSightUpdated(AActor* Actor, FAIStimulus Stimulus);
};
