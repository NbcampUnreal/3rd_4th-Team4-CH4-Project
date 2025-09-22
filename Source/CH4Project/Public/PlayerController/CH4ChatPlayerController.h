#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4ChatPlayerController.generated.h"

UCLASS()
class CH4PROJECT_API ACH4ChatPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACH4ChatPlayerController();

protected:
	virtual void BeginPlay() override;

public:
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
};
