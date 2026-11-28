#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Extraction/ExtractionTypes.h"
#include "LootItemDataAsset.generated.h"

class UTexture2D;

UCLASS(BlueprintType)
class TPS_SHOOT_API ULootItemDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	FName ItemId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	ELootRarity Rarity = ELootRarity::Common;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0"))
	int32 BaseValue = 1000;
	/** 勾选表示这是全图核心大金「以太之心」 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	bool bIsAetherCore = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loot")
	TObjectPtr<UTexture2D> Icon = nullptr;
};