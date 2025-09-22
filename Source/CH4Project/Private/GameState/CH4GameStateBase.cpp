#include "GameState/CH4GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Gamemode/CH4GameMode.h"
#include "PlayerController/CH4PlayerController.h"
#include "IngameUI/CH4UserWidget.h"

//위젯 인클루드 필요

ACH4GameStateBase::ACH4GameStateBase()
{
	MatchTypes = EMatchTypes::WaitingToStart;
	RemainingThieves = 0 ;
	RemainingPolice = 0 ;
	MatchTime = 60.f; //테스트 용으로 수정 가능
	SpawnedAI = 0;
	MaxAISpawn = 10;
}

void ACH4GameStateBase::OnRep_MatchTypes()
{
	//클라이언트에서 UI 업데이트 기능
	////추후 서버에서 관리 시 확장성 확보
}

void ACH4GameStateBase::OnRep_RemainingThieves()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) 
	{
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
		{
			if (IsValid(MyPC->MyHUDWidget))
			{
				MyPC->MyHUDWidget->UpdateRemainingThieves(RemainingThieves);
			}
		}
	}
}

void ACH4GameStateBase::OnRep_RemainingPolice()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) 
	{
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
		{
			if (IsValid(MyPC->MyHUDWidget))
			{
				MyPC->MyHUDWidget->UpdateRemainingPolice(RemainingPolice);
			}
		}
	}
}

void ACH4GameStateBase::OnRep_MatchTime()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) 
	{
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
		{
			if (IsValid(MyPC->MyHUDWidget))
			{
				MyPC->MyHUDWidget->UpdateMatchTime(MatchTime);
			}
		}
	}
}


void ACH4GameStateBase::SetMatchState(EMatchTypes NewMatchType)
{
	if (HasAuthority())
	{
		MatchTypes = NewMatchType; OnRep_MatchTypes();
	} else
	{
		ServerSetMatchState(NewMatchType);
	}
}

void ACH4GameStateBase::ServerSetMatchState_Implementation(EMatchTypes NewMatchType)
{
	MatchTypes = NewMatchType; OnRep_MatchTypes();
}

bool ACH4GameStateBase::ServerSetMatchState_Validate(EMatchTypes NewMatchType)
{
	// 필요하다면 유효성 검사 추가 가능
	return true;
}

//최종적으로 승리한 역할군으로 업데이트
void ACH4GameStateBase::SetFinalResult(EWinTeam NewResult)
{
	if (HasAuthority())
	{
		FinalResult = NewResult;
		OnRep_FinalResult(); // 서버에서도 즉시 반영
	}
	else return;
}

void ACH4GameStateBase::OnRep_FinalResult()
{
	// 클라에서 UI 갱신 등 처리 가능
}

void ACH4GameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACH4GameStateBase, MatchTypes);
	DOREPLIFETIME(ACH4GameStateBase, RemainingThieves);
	DOREPLIFETIME(ACH4GameStateBase, RemainingPolice);
	DOREPLIFETIME(ACH4GameStateBase, MatchTime);
	DOREPLIFETIME(ACH4GameStateBase, SpawnedAI);
	DOREPLIFETIME(ACH4GameStateBase, MaxAISpawn);
	DOREPLIFETIME(ACH4GameStateBase, FinalResult);
}
