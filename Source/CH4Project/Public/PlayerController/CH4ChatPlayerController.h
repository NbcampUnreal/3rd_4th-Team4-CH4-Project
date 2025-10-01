#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4ChatPlayerController.generated.h"

class UCH4ChatUserWidget;

UCLASS()
class CH4PROJECT_API ACH4ChatPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACH4ChatPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// 로비 화면
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCH4ChatUserWidget> LobbyWidgetClass;

	// 로비 플레이어 리스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> LobbyProfileClass;

	// 결과 화면
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> ResultScreen;

	// 준비 확인
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bNewReady);

	// 서버에서 준비 확인
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetReady(bool bNewReady);

	// 로비 플레이어 리스트 갱신
	UFUNCTION(Client, Reliable)
	void RefreshPlayerList();

	// 이름 복제 리스트 콜백
	UFUNCTION()
	void HandleAnyPlayerNameUpdated();
	
	// 준비 UI 반영
	UFUNCTION()
	void HandleAnyReadyChanged(bool bNewReady);

	// 로비 마우스 커서 분별
	UFUNCTION(Client, Reliable)
	void SetLobbyInput();

	// 인게임 마우스 커서 분별
	UFUNCTION(Client, Reliable)
	void SetInGameInput();

	// 클라이언트 결과 화면
	// bIsWin == true, false
	UFUNCTION(Client, Reliable)
	void ShowResultScreen(bool bIsWin);

	// 로비 복귀를 서버에 요청 (클라→서버)
	UFUNCTION(Server, Reliable)
	void Server_RequestReturnLobby();

	// 로비로 돌아가기
	UFUNCTION(BlueprintCallable)
	void ReturnLobby();

	// 개인 클라이언트 게임 종료
	UFUNCTION(BlueprintCallable, Category = "Game")
	void ExitGame();

private:
	bool bPrevMatchEnded = false;
	bool bCachedResult = false;
};
