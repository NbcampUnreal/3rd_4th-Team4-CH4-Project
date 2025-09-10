// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/MainPlayerState.h"
#include "Net/UnrealNetwork.h"
AMainPlayerState::AMainPlayerState()
{
	PlayerRole = EPlayerRole::Unassigned ;// 초기값
	CurrentArrests = 0;
	MaxArrests = 0;
}
void AMainPlayerState::SetPlayerRole(EPlayerRole NewRole)
{
	if (HasAuthority())
	{
		PlayerRole = NewRole;
	}
	else
	{
		ServerSetPlayerRole(NewRole);
	}
}
void AMainPlayerState::ServerSetPlayerRole_Implementation(EPlayerRole NewRole)
{
	PlayerRole = NewRole;
}

bool AMainPlayerState::ServerSetPlayerRole_Validate(EPlayerRole NewRole)
{
	return true;
}
void AMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(AMainPlayerState, PlayerRole);
}