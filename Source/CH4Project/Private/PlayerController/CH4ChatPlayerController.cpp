#include "PlayerController/CH4ChatPlayerController.h"
#include "GameState/CH4ChatGameState.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


ACH4ChatPlayerController::ACH4ChatPlayerController()
{
	bReplicates = true;
}

bool ACH4ChatPlayerController::Server_SendChatMessage_Validate(const FString& Message)
{
	if (Message.Len() == 0) return false;
	if (Message.Len() > 512) return false;
	return true;
}

void ACH4ChatPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
	FString SenderName = TEXT("Unknown");
	ACH4ChatPlayerState* PS = GetPlayerState<ACH4ChatPlayerState>();
	if (PS)
	{
		SenderName = PS->GetPlayerName();
	}


	ACH4ChatGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACH4ChatGameState>() : nullptr;
	if (GS)
	{
		GS->Multicast_BroadcastChat(SenderName, Message);
	}
}

void ACH4ChatPlayerController::Client_ReceiveChatMessage_Implementation(const FString& SenderPlayerName, const FString& Message)
{
	OnChatMessageReceived.Broadcast(SenderPlayerName, Message);
	DisplayChatLocally(SenderPlayerName, Message);
}

void ACH4ChatPlayerController::DisplayChatLocally(const FString& Sender, const FString& Message)
{
	if (GEngine && IsLocalController())
	{
		const FString Full = FString::Printf(TEXT("%s: %s"), *Sender, *Message);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, Full);
	}
}

void ACH4ChatPlayerController::SendChatMessage(const FString& Message)
{
	if (IsLocalController() && !Message.IsEmpty())
	{
		Server_SendChatMessage(Message);
	}
}