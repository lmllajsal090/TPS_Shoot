// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Extraction/ExtractionTypes.h"
#include "LootInventoryComponent.generated.h"

class ULootItemDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLootInventoryUpdated, int32, NewTotalValue);

/**
 * 玩家轻量级战利品背包组件（挂载于 ATPS_ShootCharacter）。
 * - LootItems 使用 COND_OwnerOnly 复制，仅向物品所有者同步清单明细，节省联机带宽；
 * - TotalLootValue 与 bCarryingAetherCore 向全员复制，供队友展示与撤离结算汇总。
 */
UCLASS(ClassGroup = (Extraction), meta = (BlueprintSpawnableComponent))
class TPS_SHOOT_API ULootInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULootInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 本地 HUD 订阅该委托，在背包内容或总价值变化时刷新 UI（零 Tick 轮询） */
	UPROPERTY(BlueprintAssignable, Category = "Loot")
	FOnLootInventoryUpdated OnInventoryUpdated;

	// ==================== 客户端 & 蓝图只读查询 ====================

	UFUNCTION(BlueprintPure, Category = "Loot")
	const TArray<FLootItemEntry>& GetLootItems() const { return LootItems; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetTotalLootValue() const { return TotalLootValue; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool IsCarryingAetherCore() const { return bCarryingAetherCore; }

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool IsEmpty() const { return LootItems.Num() == 0; }

	// ==================== 服务端权威写入接口 ====================

	/** 服务端添加单个战利品（同 ItemData 自动合并数量） */
	void AddLootItem(ULootItemDataAsset* ItemData, int32 Quantity = 1);

	/** 服务端批量添加战利品（如开箱/舔包），全部入包后再统一重算与广播 */
	void AddLootItems(const TArray<FLootItemEntry>& Entries);

	/**
	 * 服务端导出所有可掉落战利品并清空背包（用于玩家阵亡/断线生成掉落包）。
	 * @param bExcludeAetherCore 默认排除以太之心，因为以太之心由 AAetherCoreActor 独立贴地掉落。
	 */
	TArray<FLootItemEntry> ExtractAllItemsForDrop(bool bExcludeAetherCore = true);

	/** 服务端移除背包中的以太之心条目（与 AAetherCoreActor::DropAtLocation 配套调用） */
	bool RemoveAetherCoreItem();

protected:
	virtual void BeginPlay() override;

private:
	/** 仅同步给 Owner，其他玩家无需感知个人明细 */
	UPROPERTY(ReplicatedUsing = OnRep_LootItems)
	TArray<FLootItemEntry> LootItems;

	/** 向全员同步，用于队友信息与全队撤离价值结算 */
	UPROPERTY(ReplicatedUsing = OnRep_TotalLootValue)
	int32 TotalLootValue = 0;

	/** 向全员同步，标记该玩家是否正携带顶级大金「以太之心」 */
	UPROPERTY(Replicated)
	bool bCarryingAetherCore = false;

	UFUNCTION()
	void OnRep_LootItems();

	UFUNCTION()
	void OnRep_TotalLootValue();

	/** 内部辅助：合并单条物品到 LootItems（不做权限校验和重算，供 AddLootItem/AddLootItems 复用） */
	void AddLootItemInternal(ULootItemDataAsset* ItemData, int32 Quantity);

	/** 服务端根据当前 LootItems 重新汇总 TotalLootValue 与 bCarryingAetherCore，并主动广播 OnInventoryUpdated */
	void RecalculateSummary();
};
