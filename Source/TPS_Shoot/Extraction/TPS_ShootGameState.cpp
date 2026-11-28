// Fill out your copyright notice in the Description page of Project Settings.


#include "Extraction/TPS_ShootGameState.h"

#include "Net/UnrealNetwork.h"

void ATPS_ShootGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ATPS_ShootGameState, MatchPhase);
	DOREPLIFETIME(ATPS_ShootGameState, FailReason);
	DOREPLIFETIME(ATPS_ShootGameState, ObjectiveCarrier);
	DOREPLIFETIME(ATPS_ShootGameState, CountdownEndServerTime);
	DOREPLIFETIME(ATPS_ShootGameState, ExtractionEndServerTime);
	DOREPLIFETIME(ATPS_ShootGameState, ExtractionHoldDuration);
	DOREPLIFETIME(ATPS_ShootGameState, SquadExtractedTotalValue);
}

float ATPS_ShootGameState::GetRemainingCountdownSeconds() const
{
	if (CountdownEndServerTime <= 0.f
		|| MatchPhase == EExtractionMatchPhase::Failed
		|| MatchPhase == EExtractionMatchPhase::Extracted)
	{
		return 0.f;
	}
	return FMath::Max(0.f, CountdownEndServerTime - static_cast<float>(GetServerWorldTimeSeconds()));
}

float ATPS_ShootGameState::GetRemainingExtractionSeconds() const
{
	if (MatchPhase == EExtractionMatchPhase::Extracting && ExtractionEndServerTime > 0.f)
	{
		return FMath::Max(0.f, ExtractionEndServerTime - static_cast<float>(GetServerWorldTimeSeconds()));
	}
	return 0.f;
}

void ATPS_ShootGameState::SetMatchPhase(EExtractionMatchPhase NewPhase, EExtractionFailReason NewFailReason)
{
	if (!HasAuthority()) { return; };
	
	if (NewFailReason==FailReason&&NewPhase==MatchPhase)
	{
		return;
	}
	
	FailReason =NewFailReason;
	MatchPhase = NewPhase;
	OnMatchPhaseChanged.Broadcast(MatchPhase, FailReason);
}

void ATPS_ShootGameState::SetObjectiveCarrier(ATPS_ShootCharacter* NewCarrier)
{
	if (!HasAuthority()) { return; };
	
	if (ObjectiveCarrier == NewCarrier) return;
	ObjectiveCarrier = NewCarrier; 
	OnObjectiveCarrierChanged.Broadcast(ObjectiveCarrier);
}

void ATPS_ShootGameState::SetCountdownEndServerTime(float NewEndServerTime)
{
	if (!HasAuthority()) { return; };
	ExtractionEndServerTime = NewEndServerTime;
}

void ATPS_ShootGameState::SetExtractionWindow(float NewEndServerTime, float NewHoldDuration)
{
	if (!HasAuthority()) { return; };
	ExtractionHoldDuration = NewHoldDuration;
	ExtractionEndServerTime = NewEndServerTime;
}

void ATPS_ShootGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase, FailReason);
}

void ATPS_ShootGameState::OnRep_ObjectiveCarrier()
{
	OnObjectiveCarrierChanged.Broadcast(ObjectiveCarrier);
}

void ATPS_ShootGameState::ClearExtractionWindow()
{
	if (!HasAuthority()) { return; };

	ExtractionEndServerTime = 0.f;
	ExtractionHoldDuration = 0.f;
}

void ATPS_ShootGameState::SetSquadExtractedTotalValue(int32 NewTotalValue)
{
	if (!HasAuthority()) { return; };
	
	if (SquadExtractedTotalValue==NewTotalValue)return;
	
	SquadExtractedTotalValue=NewTotalValue;
}

float ATPS_ShootGameState::GetExtractionProgress() const
{
	if (MatchPhase != EExtractionMatchPhase::Extracting || ExtractionHoldDuration <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	
	return FMath::Clamp(1.f - (GetRemainingExtractionSeconds() / ExtractionHoldDuration), 0.f, 1.f);;
}
