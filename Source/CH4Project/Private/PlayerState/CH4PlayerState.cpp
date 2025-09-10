// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/CH4PlayerState.h"
#include "Net/UnrealNetwork.h"
ACH4PlayerState::ACH4PlayerState()
{
	PlayerRole = EPlayerRole::Unassigned ;// 초기값
	CurrentArrests = 0;
	MaxArrests = 0;
}
void ACH4PlayerState::SetPlayerRole(EPlayerRole NewRole)
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
void ACH4PlayerState::ServerSetPlayerRole_Implementation(EPlayerRole NewRole)
{
	PlayerRole = NewRole;
}

bool ACH4PlayerState::ServerSetPlayerRole_Validate(EPlayerRole NewRole)
{
	return true;
}


void ACH4PlayerState::OnRep_PlayerRole()
{
	// 클라이언트에서 역할이 변경될 때 UI/애니메이션 처리 가능
	UE_LOG(LogTemp, Log, TEXT("OnRep_PlayerRole: 내 역할 %d"), (int32)PlayerRole);
	// 예: RoleWidget->UpdateRole(PlayerRole);
}

void ACH4PlayerState::SetCurrentArrests(int32 NewCurrentArrests)
{
	if (HasAuthority())
	{
		CurrentArrests = NewCurrentArrests;
		// 서버에서 값 변경 → 클라이언트에서 OnRep_CurrentArrests 호출
	}
}

void ACH4PlayerState::SetMaxArrests(int32 NewMaxArrests)
{
	if (HasAuthority())
	{
		MaxArrests = NewMaxArrests;
		// 서버에서 값 변경 → 클라이언트에서 OnRep_MaxArrests 호출
	}
}

void ACH4PlayerState::OnRep_CurrentArrests()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_CurrentArrests: 현재 체포 %d"), CurrentArrests);
	// UI/위젯 즉시 갱신
	// RoleWidget->UpdateCurrentArrests(CurrentArrests);
}

void ACH4PlayerState::OnRep_MaxArrests()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_MaxArrests: 최대 체포 %d"), MaxArrests);
	// UI/위젯 즉시 갱신
	// RoleWidget->UpdateMaxArrests(MaxArrests);
}


void ACH4PlayerState::ClientReceiveRole_Implementation(EPlayerRole NewRole)
{
	PlayerRole = NewRole; // 로컬에서도 바로 반영
	UE_LOG(LogTemp, Log, TEXT("ClientReceiveRole: 내 역할 %d"), (int32)PlayerRole);
	// UI/위젯 즉시 갱신 가능
	// RoleWidget->UpdateRole(PlayerRole);
}


void ACH4PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 역할 Replication
	DOREPLIFETIME(ACH4PlayerState, PlayerRole);

	// 체포 횟수 Replication
	DOREPLIFETIME(ACH4PlayerState, CurrentArrests);
	DOREPLIFETIME(ACH4PlayerState, MaxArrests);
}