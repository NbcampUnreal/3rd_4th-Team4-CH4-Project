#include "PlayerState/CH4PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerController/CH4PlayerController.h"
#include "GameState/CH4GameStateBase.h"
#include "IngameUI/CH4UserWidget.h"

ACH4PlayerState::ACH4PlayerState()
{
	PlayerRole = EPlayerRole::Unassigned ; // 초기값
	RemainingArrests = 3; //동적 체포 X, MaxArrests는 굳이 수정할 필요 없음.
	MaxArrests = 3;
}

void ACH4PlayerState::BeginPlay()
{
	UE_LOG(LogTemp, Display, TEXT("RemainingArrests : %d"), RemainingArrests);
}

void ACH4PlayerState::SetPlayerRole(EPlayerRole NewRole)
{
	if (HasAuthority())
	{
		if (PlayerRole != NewRole)
		{
			PlayerRole = NewRole;
		}
	}
	else
	{
		ServerSetPlayerRole(NewRole);
	}
}

void ACH4PlayerState::ServerSetPlayerRole_Implementation(EPlayerRole NewRole)
{
	if (PlayerRole != NewRole)
	{
		PlayerRole = NewRole;
	}
}

bool ACH4PlayerState::ServerSetPlayerRole_Validate(EPlayerRole NewRole)
{
	return true;
}


void ACH4PlayerState::OnRep_PlayerRole() //디버그 로그가 중첩되서 출력되는 현상 해결 로직
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (PC->IsLocalController())
		{
			// UE_LOG(LogTemp, Log, TEXT("OnRep_PlayerRole: 내 역할 %d"), (int32)PlayerRole);
			if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
			{
				if (MyPC->MyHUDWidget)
				{
					MyPC->MyHUDWidget->UpdatePlayerRole(PlayerRole);
				}
			}
		}
	}
}

void ACH4PlayerState::SetRemainingArrests(int32 NewRemainingArrests)
{
	if (HasAuthority())
	{
		RemainingArrests = NewRemainingArrests;
	}
}

void ACH4PlayerState::SetMaxArrests(int32 NewMaxArrests)
{
	if (HasAuthority())
	{
		MaxArrests = NewMaxArrests;
		RemainingArrests = MaxArrests; // 갱신될 때마다 리셋
	}
}

void ACH4PlayerState::OnRep_RemainingArrests()
{
	//UE_LOG(LogTemp, Log, TEXT("OnRep_RemainingArrests: 남은 체포 %d"), RemainingArrests);
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) 
	{
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
		{
			if (MyPC->MyHUDWidget)
			{
				MyPC->MyHUDWidget->UpdateRemainingArrests(RemainingArrests);
			}
		}
	}
}

void ACH4PlayerState::OnRep_MaxArrests()
{
	UE_LOG(LogTemp, Log, TEXT("OnRep_MaxArrests: 최대 체포 %d"), MaxArrests);
}


void ACH4PlayerState::ClientReceiveRole_Implementation(EPlayerRole NewRole)
{
	PlayerRole = NewRole; // 로컬에서도 바로 반영
	UE_LOG(LogTemp, Log, TEXT("ClientReceiveRole: 내 역할 %d"), (int32)PlayerRole);
	// 추후 해당되는 위젯 파트에  추가 가능.
}

void ACH4PlayerState::UpdateKillFeedUI_Implementation(const FString& KillerName, const FString& VictimName)
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
		{
			if (MyPC->MyHUDWidget)
			{
				MyPC->MyHUDWidget->AddKillFeedEntry(KillerName, VictimName);
			}
		}
	}
}

void ACH4PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 역할 Replication
	DOREPLIFETIME(ACH4PlayerState, PlayerRole);

	// 체포 횟수 Replication
	DOREPLIFETIME(ACH4PlayerState, RemainingArrests);
	DOREPLIFETIME(ACH4PlayerState, MaxArrests);

}
