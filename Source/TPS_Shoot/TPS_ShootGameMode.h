// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * 职责：项目的 GameMode 基类入口。目前尚未在此类中实现具体的对局规则。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPS_ShootGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ATPS_ShootGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ATPS_ShootGameMode();
};



