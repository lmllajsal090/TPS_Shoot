// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ATPSHUD.h"

#include "UTPSMainHUD.h"

void AATPSHUD::BeginPlay()
{
	Super::BeginPlay();
	
	APlayerController* PC=GetOwningPlayerController();
	if (PC&&PC->IsLocalController())
	{
		if (MainHUDClass&&!MainHUD)
		{
			MainHUD=CreateWidget<UUTPSMainHUD>(PC,MainHUDClass);
			if (MainHUD)
			{
				MainHUD->AddToViewport();
			}
		}
	}
}
