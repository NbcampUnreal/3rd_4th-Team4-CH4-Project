// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/MainPlayerState.h"
#include "Net/UnrealNetwork.h"


AMainPlayerState::AMainPlayerState()
{
	PlayerRole = EPlayerRole::Unassigned ; // 초기값
}

void AMainPlayerState::SetPlayerRole(EPlayerRole NewRole)
{
	PlayerRole = NewRole;
}

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainPlayerState, PlayerRole);
}
