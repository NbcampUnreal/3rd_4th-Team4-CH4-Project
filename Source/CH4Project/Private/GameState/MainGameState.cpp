// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/MainGameState.h"
#include "Net/UnrealNetwork.h"


AMainGameState::AMainGameState()
{
	MatchTypes = EMatchTypes::WaitingToStart;
	RemainingThieves = 0 ;
	MatchTime = 600.f;
}

void AMainGameState::OnRep_MatchTypes()
{
	//배치 상태 변경을 위한 코드
	// 클라이언트에서 UI 업데이트 가능

}

void AMainGameState::SetMatchState(EMatchTypes NewMatchType)
{
	MatchTypes = NewMatchType;
	OnRep_MatchTypes();
}

void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainGameState, MatchTypes);
	DOREPLIFETIME(AMainGameState, RemainingThieves);
	DOREPLIFETIME(AMainGameState, MatchTime);

}

