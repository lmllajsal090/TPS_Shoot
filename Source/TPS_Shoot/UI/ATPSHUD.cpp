// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ATPSHUD.h"

#include "GameFramework/PlayerController.h"
#include "TPS_ShootCharacter.h"
#include "UI/UTPSMainHUD.h"

void AATPSHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!IsValid(PC) || !PC->IsLocalController() || !MainHUDClass)
	{
		return;
	}

	if (!IsValid(MainHUD))
	{
		MainHUD = CreateWidget<UUTPSMainHUD>(PC, MainHUDClass);
		if (!IsValid(MainHUD))
		{
			return;
		}

		MainHUD->AddToViewport();
	}

	// Listen for future Pawn changes, then initialize from the current Pawn.
	PC->OnPossessedPawnChanged.AddUniqueDynamic(this, &AATPSHUD::OnPossessedPawnChanged);
	OnPossessedPawnChanged(nullptr, PC->GetPawn());
}

void AATPSHUD::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(MainHUD))
	{
		MainHUD->SetObservedCharacter(Cast<ATPS_ShootCharacter>(NewPawn));
	}
}

void AATPSHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerController* PC = GetOwningPlayerController(); IsValid(PC))
	{
		PC->OnPossessedPawnChanged.RemoveDynamic(this, &AATPSHUD::OnPossessedPawnChanged);
	}

	if (IsValid(MainHUD))
	{
		// Clear child Widget data bindings before removing the HUD.
		MainHUD->SetObservedCharacter(nullptr);
		MainHUD->RemoveFromParent();
		MainHUD = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
