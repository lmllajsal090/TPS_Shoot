// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "EnemyCombatConfig.generated.h"

class UAnimMontage;

/**
 * 单个近战攻击招式的静态参数配置。
 * 玩法时序（前摇/后摇/冷却）与几何命中参数统一由服务端读取，Montage 负责客户端与主机表现。
 */
USTRUCT(BlueprintType)
struct TPS_SHOOT_API FEnemyAttackSpec
{
	GENERATED_BODY()

	/** 攻击动作蒙太奇 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;

	/** 蒙太奇播放速率 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.1"))
	float PlayRate = 1.0f;

	/** 单次有效命中造成的伤害 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0"))
	float Damage = 15.0f;

	/** 允许发起攻击与判定命中的最大距离（cm） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "10.0", Units = "cm"))
	float AttackRange = 180.0f;

	/** 从起手到执行唯一一次命中判定的前摇时长（s） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.05", Units = "s"))
	float WindupTime = 0.35f;

	/** 命中判定完成到本轮攻击结束的后摇恢复时长（s） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.05", Units = "s"))
	float RecoveryTime = 0.45f;

	/** 本招式结束后进入下一次攻击前的冷却时间（s） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.0", Units = "s"))
	float Cooldown = 1.2f;

	/** 前方有效挥击扇形半角（度），默认 60° 对应 Dot >= 0.5 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "5.0", ClampMax = "180.0", Units = "deg"))
	float SwingHalfAngleDegrees = 60.0f;

	/** 多招式随机挑选权重 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "0.01"))
	float SelectionWeight = 1.0f;
};

/**
 * 敌人战斗配置资产（DataAsset）。
 * 将招式列表、冷却偏差、遮挡检测通道等玩法参数与 C++ 逻辑解耦。
 */
UCLASS(BlueprintType)
class TPS_SHOOT_API UEnemyCombatConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 可用的近战攻击招式列表 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Attacks")
	TArray<FEnemyAttackSpec> Attacks;

	/** 冷却随机偏差（±秒），避免多只敌人完全同步出招 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float CooldownVariation = 0.15f;

	/** 前摇阶段是否锁定面向目标；到达命中时刻后释放焦点，让后摇保持挥击朝向 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Facing")
	bool bFaceTargetDuringWindup = true;

	/** 命中遮挡校验使用的碰撞通道（用于阻挡隔墙/隔掩体命中） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|HitCheck")
	TEnumAsByte<ECollisionChannel> ObstructionTraceChannel = ECC_Visibility;

	/** 调试开关：绘制命中判定距离、扇形与遮挡射线（仅非 Shipping 构建生效） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Debug")
	bool bDrawDebugHitCheck = false;

	/** 获取当前配置中所有招式的最大攻击距离 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxAttackRange() const;

	/**
	 * 根据与目标的当前距离，从满足距离条件的招式中按权重挑选一个索引。
	 * 若存在多个候选招式，尽量避免与上一次完全重复。
	 */
	int32 SelectAttackIndex(float DistanceToTarget, int32 LastAttackIndex) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
