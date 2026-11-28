#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

class ATPS_ShootCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class TPS_SHOOT_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** 是否允许该角色当前交互（客户端用于显示提示，服务端用于权威二次校验） */
	virtual bool CanInteract(const ATPS_ShootCharacter* Interactor) const = 0;

	/** HUD 显示的提示文案，如 "搜索加密箱" / "夺取以太之心" */
	virtual FText GetInteractPrompt(const ATPS_ShootCharacter* Interactor) const = 0;

	/** 交互所需读条时长（0 = 瞬间完成，>0 = 读条搜索） */
	virtual float GetInteractDuration() const { return 0.f; }

	/** 仅服务端在校验通过/读条完成后调用 */
	virtual bool ServerExecuteInteract(ATPS_ShootCharacter* Interactor) = 0;
};