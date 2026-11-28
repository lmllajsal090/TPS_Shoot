// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：本地 HUD 的生命周期入口，创建 MainHUD 并在玩家 Pawn 切换时传递新的观察对象。
 * 职责边界：不直接计算或刷新具体血条、弹药、准星；由 MainHUD 和子 Widget 负责。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ATPSHUD.generated.h"

class APawn;
class UUTPSMainHUD;

UCLASS()
class TPS_SHOOT_API AATPSHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "UI/HUD")
	TSubclassOf<UUTPSMainHUD> MainHUDClass;

	UPROPERTY(Transient)
	TObjectPtr<UUTPSMainHUD> MainHUD;

	virtual void BeginPlay() override;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
};
