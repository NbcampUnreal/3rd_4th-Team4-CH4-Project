#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4ChatPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatMessage, const FString&, Sender, const FString&, Message);

UCLASS()
class CH4PROJECT_API ACH4ChatPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACH4ChatPlayerController();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendChatMessage(const FString& Message);
	bool Server_SendChatMessage_Validate(const FString& Message);
	void Server_SendChatMessage_Implementation(const FString& Message);

	UFUNCTION(Client, Reliable)
	void Client_ReceiveChatMessage(const FString& SenderPlayerName, const FString& Message);
	void Client_ReceiveChatMessage_Implementation(const FString& SenderPlayerName, const FString& Message);

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessage OnChatMessageReceived;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& Message);

protected:
	void DisplayChatLocally(const FString& Sender, const FString& Message);
};
