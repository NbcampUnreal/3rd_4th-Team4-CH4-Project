#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "OutGameUI/CH4ChatUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
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
    if (!IsLocalController()) return;

    bShowMouseCursor = true;
    FInputModeGameAndUI Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetHideCursorDuringCapture(false);
    SetInputMode(Mode);

    if (LobbyWidgetClass)
    {
        if (UCH4ChatUserWidget* LobbyUI = CreateWidget<UCH4ChatUserWidget>(this, LobbyWidgetClass))
        {
            LobbyUI->AddToViewport();

            FInputModeGameAndUI Mode2;
            Mode2.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Mode2.SetHideCursorDuringCapture(false);
            Mode2.SetWidgetToFocus(LobbyUI->TakeWidget());
            SetInputMode(Mode2);
        }
    }
}

// 준비 상태
void ACH4ChatPlayerController::SetReady(bool bNewReady)
{
    if (IsLocalController())
        Server_SetReady(bNewReady);
}

// 서버에서 준비 상태 확인
void ACH4ChatPlayerController::Server_SetReady_Implementation(bool bNewReady)
{
    if (ACH4ChatPlayerState* PS = GetPlayerState<ACH4ChatPlayerState>())
        PS->ServerSetReady(bNewReady);
}

void ACH4ChatPlayerController::ShowResultScreen_Implementation(bool bIsWin)
{
    if (!ResultScreen) return;

    if (UUserWidget* ResultUI = CreateWidget<UUserWidget>(this, ResultScreen))
    {
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

        bShowMouseCursor = true;
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetWidgetToFocus(ResultUI->TakeWidget());
        SetInputMode(Mode);
    }
}



// 로비로 복귀
void ACH4ChatPlayerController::ReturnLobby()
{
    UGameplayStatics::OpenLevel(this, FName("LobbyMap"));
}

// 게임 종료
void ACH4ChatPlayerController::ExitGame()
{
    if (UWorld* World = GetWorld())
    {
        UKismetSystemLibrary::QuitGame(World, this, EQuitPreference::Quit, false);
    }
}