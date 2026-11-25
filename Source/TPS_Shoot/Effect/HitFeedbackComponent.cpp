// Fill out your copyright notice in the Description page of Project Settings.


#include "Effect/HitFeedbackComponent.h"

UHitFeedbackComponent::UHitFeedbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UHitFeedbackComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


