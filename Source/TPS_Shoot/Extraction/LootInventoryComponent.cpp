// Fill out your copyright notice in the Description page of Project Settings.


#include "Extraction/LootInventoryComponent.h"
#include "Extraction/LootItemDataAsset.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
ULootInventoryComponent::ULootInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


void ULootInventoryComponent::AddLootItem(ULootItemDataAsset* ItemData, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return; 
	}
	if (!IsValid(ItemData) || Quantity <= 0) { return; }
	
	AddLootItemInternal(ItemData, Quantity);
	RecalculateSummary();
}


void ULootInventoryComponent::OnRep_LootItems()
{
	OnInventoryUpdated.Broadcast(TotalLootValue);
}

void ULootInventoryComponent::OnRep_TotalLootValue()
{
	OnInventoryUpdated.Broadcast(TotalLootValue);
}

void ULootInventoryComponent::AddLootItemInternal(ULootItemDataAsset* ItemData, int32 Quantity)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	if (!IsValid(ItemData) || Quantity <= 0){return;}
	
	for (FLootItemEntry& Entry : LootItems)
	{
		if (Entry.ItemData == ItemData )
		{
			Entry.Quantity+=Quantity;
			return;
		}
	}
	
	FLootItemEntry NewEntry;
	NewEntry.ItemData=ItemData;
	NewEntry.Quantity=Quantity;
	LootItems.Add(NewEntry);
}

void ULootInventoryComponent::RecalculateSummary()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}
	
	TotalLootValue = 0;
	bCarryingAetherCore = false;
	
	for (const FLootItemEntry& Entry : LootItems)
	{
		TotalLootValue+=Entry.Quantity*Entry.ItemData->BaseValue;
		
		if (!IsValid(Entry.ItemData) || Entry.Quantity <= 0)
		{
			continue;
		}
		
		if (Entry.ItemData->bIsAetherCore)
		{
			bCarryingAetherCore=true;
		}
	}
	OnInventoryUpdated.Broadcast(TotalLootValue);
}

bool ULootInventoryComponent::RemoveAetherCoreItem()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	const int32 RemovedCount = LootItems.RemoveAll([](const FLootItemEntry& Entry)
	{
		return Entry.ItemData && Entry.ItemData->bIsAetherCore;
	});

	if (RemovedCount > 0)
	{
		RecalculateSummary();
		return true;
	}

	return false;
}

void ULootInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULootInventoryComponent::AddLootItems(const TArray<FLootItemEntry>& Entries)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return; 
	}
	
	if (Entries.IsEmpty())
	{
		return;
	}

	for (const FLootItemEntry& Entry : Entries)
	{
		AddLootItemInternal(Entry.ItemData, Entry.Quantity);
	}

	RecalculateSummary();
}

TArray<FLootItemEntry> ULootInventoryComponent::ExtractAllItemsForDrop(bool bExcludeAetherCore)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return {};
	}
	
	TArray<FLootItemEntry> DroppedItems;
	
	for (const FLootItemEntry& Entry : LootItems)
	{
		if (!IsValid(Entry.ItemData) || Entry.Quantity <= 0)
		{
			continue;
		}
		if (bExcludeAetherCore && Entry.ItemData->bIsAetherCore)
		{
			continue;
		}
		DroppedItems.Add(Entry);
	}
	LootItems.Reset();
	
	RecalculateSummary();
	return DroppedItems;
	
}


void ULootInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ULootInventoryComponent, LootItems, COND_OwnerOnly);
	DOREPLIFETIME(ULootInventoryComponent,bCarryingAetherCore);
	DOREPLIFETIME(ULootInventoryComponent, TotalLootValue);
}




