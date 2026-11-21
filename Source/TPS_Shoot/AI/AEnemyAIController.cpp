// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AEnemyAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AAEnemyAIController::AAEnemyAIController()
{
	Blackboard=CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent=CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);
	
}

void AAEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunBehaviorTree(BehaviorTree);
}
