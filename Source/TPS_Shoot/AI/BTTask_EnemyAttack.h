// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_EnemyAttack.generated.h"

class UEnemyCombatComponent;

/** 每个行为树运行实例独享的任务内存，避免多只敌人共用节点对象时互相覆盖状态 */
struct FBTEnemyAttackMemory
{
	TWeakObjectPtr<UEnemyCombatComponent> CombatComponent;
	FDelegateHandle AttackFinishedHandle;
};

/**
 * 通用敌人攻击行为树任务节点。
 * 职责极简：向 UEnemyCombatComponent 请求发起攻击，进入 InProgress 状态等待完成或被行为树中止。
 */
UCLASS()
class TPS_SHOOT_API UBTTask_EnemyAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_EnemyAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTEnemyAttackMemory); }
	virtual FString GetStaticDescription() const override;

protected:
	/** 黑板中目标 Actor 的键（默认匹配 BB_Enemy 的 TargetActor） */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

private:
	void ResetAttackBinding(FBTEnemyAttackMemory* Memory) const;
};
