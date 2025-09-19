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

protected:
	virtual void BeginPlay() override;

public:
	// 클라이언트 -> 서버 채팅 메시지 전송
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendChatMessage(const FString& Message);
	bool Server_SendChatMessage_Validate(const FString& Message);
	void Server_SendChatMessage_Implementation(const FString& Message);

	// 서버 -> 클라이언트 채팅 메시지 수신
	UFUNCTION(Client, Reliable)
	void Client_ReceiveChatMessage(const FString& SenderPlayerName, const FString& Message);
	void Client_ReceiveChatMessage_Implementation(const FString& SenderPlayerName, const FString& Message);

	// 블루프린트에서 메시지를 UI에 표시
	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FOnChatMessage OnChatMessageReceived;

	// 블루프린트에서 호출할 수 있는 함수
	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& Message);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LobbyWidgetClass;

	// 결과 화면 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> ResultScreen;

	// 준비 확인
	UFUNCTION(Server, Reliable)
	void Server_SetReady(bool bNewReady);

	// 개인 클라이언트 게임 종료
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ExitGame();

	// 클라이언트 결과 화면
	// bIsWin == true, false
	UFUNCTION(Client, Reliable)
	void ShowResultScreen(bool bIsWin);

	// 로비로 돌아가기
	UFUNCTION(BlueprintCallable)
	void ReturnLobby();

protected:
	void DisplayChatLocally(const FString& Sender, const FString& Message);
};
