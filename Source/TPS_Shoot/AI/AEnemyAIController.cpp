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
	const FName DistanceKey(TEXT("Distance"));
}

AAEnemyAIController::AAEnemyAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);

	UAIPerceptionComponent* Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	Perception->ConfigureSense(*SightConfig);
	Perception->SetDominantSense(UAISense_Sight::StaticClass());
	SetPerceptionComponent(*Perception);
}

void AAEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 同步蓝图子类中覆盖的视觉参数到 SightConfig
	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		if (IsValid(SightConfig))
		{
			SightConfig->SightRadius = SightRadius;
			SightConfig->LoseSightRadius = FMath::Max(LoseSightRadius, SightRadius);
			SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
			Perception->ConfigureSense(*SightConfig);
		}
	}

	if (IsValid(BehaviorTree) && RunBehaviorTree(BehaviorTree))
	{
		if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
		{
			Perception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AAEnemyAIController::HandleSightUpdated);
		}
	}
}

void AAEnemyAIController::OnUnPossess()
{
	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAEnemyAIController::HandleSightUpdated);
		Perception->ForgetAll();
	}

	Super::OnUnPossess();
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetActorKey);
	}
}

void AAEnemyAIController::NotifyDamagedBy(AController* InstigatedBy)
{
	const AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn());
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!IsValid(Enemy) || Enemy->IsDead() || !IsValid(BlackboardComponent) || !IsValid(InstigatedBy))
	{
		return;
	}

	ATPS_ShootCharacter* Attacker = Cast<ATPS_ShootCharacter>(InstigatedBy->GetPawn());
	if (!IsValid(Attacker) || !Attacker->IsPlayerControlled() || Attacker->IsDead())
	{
		return;
	}

	const ATPS_ShootCharacter* CurrentTarget = Cast<ATPS_ShootCharacter>(BlackboardComponent->GetValueAsObject(TargetActorKey));
	if (ShouldSwitchTarget(Enemy, CurrentTarget, Attacker))
	{
		SetTargetPlayer(Enemy, Attacker);
	}
}

void AAEnemyAIController::StopAIOnDeath()
{
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	ClearFocus(EAIFocusPriority::Default);

	if (UAIPerceptionComponent* Perception = GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.RemoveDynamic(this, &AAEnemyAIController::HandleSightUpdated);
		Perception->ForgetAll();
	}

	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->ClearValue(TargetActorKey);
	}

	if (UBrainComponent* Brain = GetBrainComponent())
	{
		Brain->StopLogic(TEXT("EnemyDead"));
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
		if (ShouldSwitchTarget(Enemy, CurrentTarget, Player))
		{
			SetTargetPlayer(Enemy, Player);
		}
	}
	else if (CurrentTarget == Player)
	{
		BlackboardComponent->ClearValue(TargetActorKey);
	}
}

bool AAEnemyAIController::ShouldSwitchTarget(const AEnemyBase* Enemy, const ATPS_ShootCharacter* CurrentTarget,
	const ATPS_ShootCharacter* CandidateTarget) const
{
	if (!IsValid(Enemy) || !IsValid(CandidateTarget) || CandidateTarget->IsDead())
	{
		return false;
	}

	if (!IsValid(CurrentTarget) || CurrentTarget->IsDead())
	{
		return true;
	}

	if (CurrentTarget == CandidateTarget)
	{
		return false;
	}

	// 多人仇恨防抖：只有新目标明显更近时才切换，避免在两名玩家间反复横跳
	const float CurrentDist = Enemy->GetDistanceTo(CurrentTarget);
	const float CandidateDist = Enemy->GetDistanceTo(CandidateTarget);
	return CandidateDist <= CurrentDist * TargetSwitchDistanceRatio
		&& (CurrentDist - CandidateDist) >= TargetSwitchDistanceAdvantage;
}

void AAEnemyAIController::SetTargetPlayer(const AEnemyBase* Enemy, ATPS_ShootCharacter* NewTarget)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (!IsValid(BlackboardComponent) || !IsValid(Enemy) || !IsValid(NewTarget))
	{
		return;
	}

	BlackboardComponent->SetValueAsFloat(DistanceKey, Enemy->GetDistanceTo(NewTarget));
	BlackboardComponent->SetValueAsObject(TargetActorKey, NewTarget);
}
