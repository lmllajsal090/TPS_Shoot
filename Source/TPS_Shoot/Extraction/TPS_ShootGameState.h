#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Extraction/ExtractionTypes.h"
#include "TPS_ShootCharacter.h"
#include "TPS_ShootGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnExtractionPhaseChanged,
	EExtractionMatchPhase, NewPhase,
	EExtractionFailReason, FailReason);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnObjectiveCarrierChanged,
	ATPS_ShootCharacter*, NewCarrier);

/**
 * 搜打撤全局对局状态（与 ATPS_ShootGameMode : AGameModeBase 成对，必须继承 AGameStateBase）。
 * 倒计时与撤离读条只复制一次“截止服务器时间戳”，客户端本地用 GetServerWorldTimeSeconds() 实时插值。
 */
UCLASS()
class TPS_SHOOT_API ATPS_ShootGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "Extraction")
	FOnExtractionPhaseChanged OnMatchPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Extraction")
	FOnObjectiveCarrierChanged OnObjectiveCarrierChanged;

	// ==================== 客户端 & 蓝图只读查询 ====================

	UFUNCTION(BlueprintPure, Category = "Extraction")
	EExtractionMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "Extraction")
	EExtractionFailReason GetFailReason() const { return FailReason; }

	UFUNCTION(BlueprintPure, Category = "Extraction")
	ATPS_ShootCharacter* GetObjectiveCarrier() const { return ObjectiveCarrier; }

	UFUNCTION(BlueprintPure, Category = "Extraction")
	bool HasObjectiveCarrier() const { return IsValid(ObjectiveCarrier); }

	UFUNCTION(BlueprintPure, Category = "Extraction")
	int32 GetSquadExtractedTotalValue() const { return SquadExtractedTotalValue; }

	UFUNCTION(BlueprintPure, Category = "Extraction")
	float GetRemainingCountdownSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Extraction")
	float GetRemainingExtractionSeconds() const;

	/** 返回 0.0 ~ 1.0，供撤离进度条直接使用 */
	UFUNCTION(BlueprintPure, Category = "Extraction")
	float GetExtractionProgress() const;

	// ==================== 服务端权威写入接口（由 GameMode 调用） ====================

	void SetMatchPhase(EExtractionMatchPhase NewPhase, EExtractionFailReason NewFailReason = EExtractionFailReason::None);
	void SetObjectiveCarrier(ATPS_ShootCharacter* NewCarrier);
	void SetCountdownEndServerTime(float NewEndServerTime);
	void SetExtractionWindow(float NewEndServerTime, float NewHoldDuration);
	void ClearExtractionWindow();
	void SetSquadExtractedTotalValue(int32 NewTotalValue);

private:
	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	EExtractionMatchPhase MatchPhase = EExtractionMatchPhase::Exploring;

	UPROPERTY(Replicated)
	EExtractionFailReason FailReason = EExtractionFailReason::None;

	UPROPERTY(ReplicatedUsing = OnRep_ObjectiveCarrier)
	TObjectPtr<ATPS_ShootCharacter> ObjectiveCarrier = nullptr;

	/** 拿到以太之心后的全局倒计时截止时间（ServerWorldTimeSeconds） */
	UPROPERTY(Replicated)
	float CountdownEndServerTime = 0.f;

	/** 进入撤离区后的读条截止时间（ServerWorldTimeSeconds） */
	UPROPERTY(Replicated)
	float ExtractionEndServerTime = 0.f;

	/** 本次撤离读条总时长（用于算 0~1 进度百分比） */
	UPROPERTY(Replicated)
	float ExtractionHoldDuration = 0.f;

	/** 撤离成功时全队存活成员带出的战利品总价值 */
	UPROPERTY(Replicated)
	int32 SquadExtractedTotalValue = 0;

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_ObjectiveCarrier();
};