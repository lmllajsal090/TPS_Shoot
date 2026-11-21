// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/AEnemyAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBase.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TPS_ShootCharacter.h"

namespace
{
	const FName TargetActorKey(TEXT("TargetActor"));
}

AAEnemyAIController::AAEnemyAIController()
{
	Blackboard=CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent=CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);

	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(UAISense_Sight::StaticClass());
	SetPerceptionComponent(*Perception);
}

void AAEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsValid(BehaviorTree) && RunBehaviorTree(BehaviorTree))
	{
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AAEnemyAIController::HandleSightUpdated);
	}
}

void AAEnemyAIController::OnUnPossess()
{
	UAIPerceptionComponent* Perception = GetPerceptionComponent();
	Perception->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAEnemyAIController::HandleSightUpdated);
	Perception->ForgetAll();

	Super::OnUnPossess();
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetActorKey);
	}
}

void AAEnemyAIController::HandleSightUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	const AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn());
	ATPS_ShootCharacter* Player = Cast<ATPS_ShootCharacter>(Actor);
	if (!BlackboardComponent || !IsValid(Enemy) || Enemy->IsDead() || !IsValid(Player))
	{
		return;
	}

	const ATPS_ShootCharacter* CurrentTarget = Cast<ATPS_ShootCharacter>(BlackboardComponent->GetValueAsObject(TargetActorKey));
	if (Stimulus.WasSuccessfullySensed() && Player->IsPlayerControlled() && !Player->IsDead())
	{
		if (!IsValid(CurrentTarget) || CurrentTarget->IsDead())
		{
			BlackboardComponent->SetValueAsObject(TargetActorKey, Player);
		}
	}
	else if (CurrentTarget == Player)
	{
		BlackboardComponent->ClearValue(TargetActorKey);
	}
}
