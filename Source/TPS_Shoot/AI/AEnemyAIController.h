// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AEnemyAIController.generated.h"

class AEnemyBase;
class ATPS_ShootCharacter;
class UAISenseConfig_Sight;
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

	/** 受击索敌：敌人受到玩家伤害时通知 AI 考虑切换/锁定该攻击者 */
	void NotifyDamagedBy(AController* InstigatedBy);

	/** 敌人死亡时立即停止行为树、感知与焦点，防止死后继续发攻击指令 */
	void StopAIOnDeath();
	
protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;
	
	UPROPERTY(BlueprintReadOnly,EditAnywhere,Category="AI|Tree")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float SightRadius = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float LoseSightRadius = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Perception")
	float PeripheralVisionAngle = 70.0f;

	/** 多人仇恨防抖：新目标必须比当前目标近到该比例以内才允许切换 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Targeting", meta=(ClampMin="0.1", ClampMax="0.99"))
	float TargetSwitchDistanceRatio = 0.8f;

	/** 多人仇恨防抖：新目标相对当前目标还必须具备的绝对距离优势（cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|Targeting", meta=(ClampMin="0.0", Units="cm"))
	float TargetSwitchDistanceAdvantage = 150.0f;
	
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UFUNCTION()
	void HandleSightUpdated(AActor* Actor, FAIStimulus Stimulus);

	bool ShouldSwitchTarget(const AEnemyBase* Enemy, const ATPS_ShootCharacter* CurrentTarget,
		const ATPS_ShootCharacter* CandidateTarget) const;
	void SetTargetPlayer(const AEnemyBase* Enemy, ATPS_ShootCharacter* NewTarget);
};
