// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "Health/HealthComponent.h"
#include "HitFeedbackComponent.generated.h"

class UHitFeedbackConfig;
class UNiagaraSystem;

/**
 * 通用生物受击反馈组件（适用于玩家与敌人）。
 * - 服务端订阅宿主 UHealthComponent::OnDamageTaken，进行频控节流并打包压缩参数
 * - 通过 NetMulticast(Unreliable) 广播到各端，依据 UHitFeedbackConfig 播放受击 Niagara 与音效
 * - 与数值扣血完全解耦，独立服务器自动跳过视听表现
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class TPS_SHOOT_API UHitFeedbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHitFeedbackComponent();

	UFUNCTION(BlueprintPure, Category = "HitFeedback")
	UHitFeedbackConfig* GetFeedbackConfig() const { return FeedbackConfig; }

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void BeginPlay() override;

	/** 受击表现配置资产（在蓝图子类如 BP_Whisper / BP_TrooperPlayer_Polished 中指定） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HitFeedback")
	TObjectPtr<UHitFeedbackConfig> FeedbackConfig;

	/** 联机压缩广播：仅 1 字节 HitFlags（Bit0=Headshot, Bit1=Fatal），不传输资产指针 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayHitFeedback(FVector_NetQuantize HitLocation, FVector_NetQuantizeNormal HitNormal, uint8 HitFlags);

private:
	static constexpr uint8 HitFlag_Headshot = 1 << 0;
	static constexpr uint8 HitFlag_Fatal = 1 << 1;

	UFUNCTION()
	void HandleDamageTaken(FDamageTakenEvent DamageEvent);

	UNiagaraSystem* ResolveHitVFX(uint8 HitFlags) const;

	TWeakObjectPtr<UHealthComponent> ObservedHealthComponent;

	/** 上次播发受击反馈的服务端世界时间，用于高频扫射/集火节流 */
	double LastFeedbackTime = -1.0;
};
