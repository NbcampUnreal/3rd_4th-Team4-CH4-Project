#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CH4ChatGameState.generated.h"


USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat")
	FString Sender;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chat")
	FString Message;
};

UCLASS()
class CH4PROJECT_API ACH4ChatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastChat(const FString& SenderPlayerName, const FString& Message);
	void Multicast_BroadcastChat_Implementation(const FString& SenderPlayerName, const FString& Message);

	UPROPERTY(ReplicatedUsing = OnRep_ChatHistory, BlueprintReadOnly, Category = "Chat")
	TArray<FChatMessage> ChatHistory;

	UFUNCTION()
	void OnRep_ChatHistory();

	void AddMessageToHistory(const FString& Sender, const FString& Message);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
