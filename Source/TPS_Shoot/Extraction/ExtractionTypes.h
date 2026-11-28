#pragma once

#include "CoreMinimal.h"
#include "ExtractionTypes.generated.h"

class ULootItemDataAsset;

UENUM(BlueprintType)
enum class EExtractionMatchPhase : uint8
{
	Exploring       UMETA(DisplayName = "Exploring"),
	CountdownActive UMETA(DisplayName = "Countdown Active"),
	Extracting      UMETA(DisplayName = "Extracting"),
	Extracted       UMETA(DisplayName = "Extracted"),
	Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EExtractionFailReason : uint8
{
	None             UMETA(DisplayName = "None"),
	AllPlayersDead   UMETA(DisplayName = "All Players Dead"),
	CountdownExpired UMETA(DisplayName = "Countdown Expired")
};

UENUM(BlueprintType)
enum class ELootRarity : uint8
{
	Common    UMETA(DisplayName = "Common"),
	Rare      UMETA(DisplayName = "Rare"),
	Epic      UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FLootItemEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
	TObjectPtr<ULootItemDataAsset> ItemData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot", meta = (ClampMin = "1"))
	int32 Quantity = 1;
};