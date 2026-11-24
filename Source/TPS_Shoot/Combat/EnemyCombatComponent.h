// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "EnemyCombatComponent.generated.h"

class AAIController;
class AEnemyBase;
class UEnemyCombatConfig;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnEnemyAttackFinished, bool /*bInterrupted*/);

/**
 * 敌人战斗执行组件（服务器权威）。
 *
 * 职责划分：
 * - 维护攻击状态机（前摇 Windup -> 唯一命中判定 -> 后摇 Recovery -> 冷却 Cooldown）
 * - 使用独立的 AttackId 与 bHitAttempted 保证每次挥击最多结算一次伤害且不会被过期定时器串台
 * - 服务端执行距离、前方扇形夹角与遮挡射线三道几何校验，通过 ApplyPointDamage 接入目标 HealthComponent
 * - 通过 NetMulticast 通知各端播放/停止对应的攻击 Montage，客户端只做表现
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TPS_SHOOT_API UEnemyCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnemyCombatComponent();

	/** 本轮攻击正常结束或被中断时广播（仅服务端触发，供 BT Task 等监听） */
	FOnEnemyAttackFinished OnAttackFinished;

	/** 服务端查询：当前是否满足向 TargetActor 发起攻击的条件 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanStartAttack(AActor* TargetActor) const;

	/** 服务端接口：尝试对目标发起一次近战攻击，成功返回 true */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryStartAttack(AActor* TargetActor);

	/** 服务端接口：强制中止当前攻击（如敌人死亡、受击硬直或行为树节点被中止） */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CancelAttack();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsOnCooldown() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxAttackRange() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UEnemyCombatConfig* GetCombatConfig() const { return CombatConfig; }

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** 敌人战斗配置资产（在蓝图子类如 BP_Whisper 中指定） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TObjectPtr<UEnemyCombatConfig> CombatConfig;

	/** 当前是否处于攻击动作周期内（从起手到后摇结束） */
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat|State")
	bool bIsAttacking = false;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackMontage(int32 AttackIndex);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopAttackMontage(float BlendOutTime);

private:
	void HandleAttackHitWindow(uint32 InAttackId);
	void HandleAttackComplete(uint32 InAttackId);

	bool IsValidAttackTarget(const AActor* TargetActor) const;
	AEnemyBase* GetOwnerEnemy() const;
	AAIController* GetOwnerAIController() const;
	void ClearAttackTimers();
	void ReleaseAttackFocus();

	/** 每次成功发起攻击时递增，取消后不复用，防止旧定时器误触发 */
	uint32 CurrentAttackId = 0;

	/** 当前正在执行的招式索引 */
	int32 ActiveAttackIndex = INDEX_NONE;

	/** 上一次选中的招式索引，用于多招式防重复 */
	int32 LastAttackIndex = INDEX_NONE;

	/** 本轮攻击起手时锁定的目标，挥击中途不随感知切换 */
	TWeakObjectPtr<AActor> LockedTarget;

	/** 本轮是否已执行或作废唯一命中判定 */
	bool bHitAttempted = false;

	/** 本轮攻击是否由本组件设置了 AI Focus */
	bool bHasAppliedAttackFocus = false;

	/** 下次允许发起攻击的世界时间 */
	double NextAllowedAttackTime = 0.0;

	FTimerHandle WindupTimerHandle;
	FTimerHandle FinishTimerHandle;
};
