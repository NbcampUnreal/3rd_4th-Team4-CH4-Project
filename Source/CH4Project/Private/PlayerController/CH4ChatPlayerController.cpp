#include "PlayerController/CH4ChatPlayerController.h"
#include "GameState/CH4ChatGameState.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


ACH4ChatPlayerController::ACH4ChatPlayerController()
{
	bReplicates = true;
}

void ACH4ChatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController() && LobbyWidgetClass)
	{
		UUserWidget* LobbyUI = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
		if (LobbyUI)
		{
			LobbyUI->AddToViewport();
		}
	}
}

// 서버 RPC 유효성 검사
bool ACH4ChatPlayerController::Server_SendChatMessage_Validate(const FString& Message)
{
	if (Message.Len() == 0) return false;
	if (Message.Len() > 512) return false;
	return true;
}

// 서버 RPC 구현
void ACH4ChatPlayerController::Server_SendChatMessage_Implementation(const FString& Message)
{
	// 플레이어 이름 가져오기
	FString SenderName = TEXT("Unknown");
	ACH4ChatPlayerState* PS = GetPlayerState<ACH4ChatPlayerState>();
	if (PS)
	{
		SenderName = PS->GetPlayerName();
	}

	// GameState에 전달
	ACH4ChatGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACH4ChatGameState>() : nullptr;
	if (GS)
	{
		GS->Multicast_BroadcastChat(SenderName, Message);
	}
}

// 클라이언트에서 메시지 수신했을 때 실행
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

// 블루프린트에서 호출할 수 있는 함수
void ACH4ChatPlayerController::SendChatMessage(const FString& Message)
{
	if (IsLocalController() && !Message.IsEmpty())
	{
		Server_SendChatMessage(Message);
	}
}

// 준비 상태 호출
void ACH4ChatPlayerController::Server_SetReady_Implementation(bool bNewReady)
{
	if (ACH4ChatPlayerState* PS = GetPlayerState<ACH4ChatPlayerState>())
	{
		PS->ServerSetReady(bNewReady);
	}
}

// 클라이언트에서 결과 화면을 띄우는 함수 구현
void ACH4ChatPlayerController::ShowResultScreen_Implementation(bool bIsWin)
{
	if (ResultScreen == nullptr)
	{
		return;
	}

	// 결과 UI 생성
	UUserWidget* ResultUI = CreateWidget<UUserWidget>(this, ResultScreen);
	if (ResultUI == nullptr)
	{
		return;
	}

	// 화면에 표시
	ResultUI->AddToViewport();

	if (UImage* ResultImage = Cast<UImage>(ResultUI->GetWidgetFromName(TEXT("ResultScreen"))))
	{
		UTexture2D* WinTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Win.Win"));
		UTexture2D* LoseTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Lose.Lose"));

		if (WinTex && LoseTex)
		{
			ResultImage->SetBrushFromTexture(bIsWin ? WinTex : LoseTex);
		}
	}
}

// 로비로 돌아가기
void ACH4ChatPlayerController::ReturnLobby()
{
	UGameplayStatics::OpenLevel(this, FName("LobbyMap"));
}

// 게임 종료
void ACH4ChatPlayerController::ExitGame()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UKismetSystemLibrary::QuitGame(World, this, EQuitPreference::Quit, false);
}