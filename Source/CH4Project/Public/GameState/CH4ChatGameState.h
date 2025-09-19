#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CH4ChatGameState.generated.h"


USTRUCT(BlueprintType)
struct FChatMessage
{
	GENERATED_BODY()

	// 보내는 사람
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Sender;

	// 보낸 메시지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chat")
	FString Message;
};

UCLASS()
class CH4PROJECT_API ACH4ChatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	// 서버에서 호출 -> 모든 클라이언트에게 메시지
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastChat(const FString& SenderPlayerName, const FString& Message);
	void Multicast_BroadcastChat_Implementation(const FString& SenderPlayerName, const FString& Message);

	// 채팅 내역
	UPROPERTY(ReplicatedUsing = OnRep_ChatHistory, BlueprintReadOnly, Category = "Chat")
	TArray<FChatMessage> ChatHistory;

	UFUNCTION()
	void OnRep_ChatHistory();

	void AddMessageToHistory(const FString& Sender, const FString& Message);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
