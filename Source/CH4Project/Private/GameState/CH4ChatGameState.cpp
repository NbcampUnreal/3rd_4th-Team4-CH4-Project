#include "GameState/CH4ChatGameState.h"
#include "PlayerController/CH4ChatPlayerController.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "Net/UnrealNetwork.h"


void ACH4ChatGameState::Multicast_BroadcastChat_Implementation(const FString& SenderPlayerName, const FString& Message)
{
	AddMessageToHistory(SenderPlayerName, Message);

	// 모든 플레이어에게 컨트롤러 제공
	UWorld* World = GetWorld();
	if (!World) return;


	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			ACH4ChatPlayerController* CH4PC = Cast<ACH4ChatPlayerController>(PC);
			if (CH4PC)
			{
				CH4PC->Client_ReceiveChatMessage(SenderPlayerName, Message);
			}
		}
	}
}

void ACH4ChatGameState::AddMessageToHistory(const FString& Sender, const FString& Message)
{
	// 새로운 메시지 내역에 추가
	FChatMessage NewMsg;
	NewMsg.Sender = Sender;
	NewMsg.Message = Message;
	ChatHistory.Add(NewMsg);
}

void ACH4ChatGameState::OnRep_ChatHistory()
{

}

void ACH4ChatGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME(ACH4ChatGameState, ChatHistory);
}