// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/BTTask_EnemyAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Combat/EnemyCombatComponent.h"
#include "EnemyBase.h"

UBTTask_EnemyAttack::UBTTask_EnemyAttack()
{
	NodeName = TEXT("Enemy Attack");
	bNotifyTaskFinished = true;

	TargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UBTTask_EnemyAttack, TargetActorKey),
		AActor::StaticClass()
	);
	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
}

EBTNodeResult::Type UBTTask_EnemyAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AEnemyBase* Enemy = AIController ? Cast<AEnemyBase>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(Enemy) || !IsValid(BlackboardComp))
	{
		return EBTNodeResult::Failed;
	}

	UEnemyCombatComponent* CombatComp = Enemy->GetCombatComponent();
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!IsValid(CombatComp) || !IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	if (!CombatComp->TryStartAttack(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	FBTEnemyAttackMemory* Memory = CastInstanceNodeMemory<FBTEnemyAttackMemory>(NodeMemory);
	ResetAttackBinding(Memory);

	Memory->CombatComponent = CombatComp;

	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp(&OwnerComp);
	Memory->AttackFinishedHandle = CombatComp->OnAttackFinished.AddWeakLambda(
		this,
		[this, WeakOwnerComp](bool bInterrupted)
		{
			if (UBehaviorTreeComponent* ValidOwnerComp = WeakOwnerComp.Get())
			{
				FinishLatentTask(*ValidOwnerComp, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
			}
		}
	);

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_EnemyAttack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTEnemyAttackMemory* Memory = CastInstanceNodeMemory<FBTEnemyAttackMemory>(NodeMemory);
	UEnemyCombatComponent* CombatComp = Memory ? Memory->CombatComponent.Get() : nullptr;

	// 先解绑委托再取消攻击，避免 CancelAttack 内部广播重入 FinishLatentTask
	ResetAttackBinding(Memory);

	if (IsValid(CombatComp))
	{
		CombatComp->CancelAttack();
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_EnemyAttack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	ResetAttackBinding(CastInstanceNodeMemory<FBTEnemyAttackMemory>(NodeMemory));
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FString UBTTask_EnemyAttack::GetStaticDescription() const
{
	return FString::Printf(TEXT("Target: %s"), *TargetActorKey.SelectedKeyName.ToString());
}

void UBTTask_EnemyAttack::ResetAttackBinding(FBTEnemyAttackMemory* Memory) const
{
	if (!Memory)
	{
		return;
	}

	if (UEnemyCombatComponent* CombatComp = Memory->CombatComponent.Get())
	{
		if (Memory->AttackFinishedHandle.IsValid())
		{
			CombatComp->OnAttackFinished.Remove(Memory->AttackFinishedHandle);
		}
	}

	Memory->AttackFinishedHandle.Reset();
	Memory->CombatComponent.Reset();
}
