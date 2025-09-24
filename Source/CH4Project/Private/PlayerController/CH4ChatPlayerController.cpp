#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "OutGameUI/CH4ChatUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/GridPanel.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"


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

// 로비 플레이어 리스트 갱신
void ACH4ChatPlayerController::RefreshPlayerList_Implementation()
{
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UCH4ChatUserWidget::StaticClass(), false);

    for (UUserWidget* Widget : FoundWidgets)
    {
        if (UGridPanel* Grid = Cast<UGridPanel>(Widget->GetWidgetFromName(TEXT("PlayerList"))))
        {
            Grid->ClearChildren();

            UWorld* World = GetWorld();
            if (!World) return;

            int32 Index = 0;
            for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
            {
                if (APlayerController* PC = It->Get())
                {
                    if (ACH4ChatPlayerState* PS = PC->GetPlayerState<ACH4ChatPlayerState>())
                    {
                        if (LobbyProfileClass)
                        {
                            if (UUserWidget* Profile = CreateWidget<UUserWidget>(this, LobbyProfileClass))
                            {
                                if (UTextBlock* NameText = Cast<UTextBlock>(Profile->GetWidgetFromName(TEXT("TextBlock_PlayerName"))))
                                {
                                    NameText->SetText(FText::FromString(PS->GetPlayerName()));
                                }
                                if (UCheckBox* ReadyBox = Cast<UCheckBox>(Profile->GetWidgetFromName(TEXT("CheckBox_Ready"))))
                                {
                                    ReadyBox->SetIsChecked(PS->IsReady());
                                }

                                Grid->AddChildToGrid(Profile, Index, 0);
                                Index++;
                            }
                        }
                    }
                }
            }
        }
    }
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

// 마우스 포인터 로비 전용
void ACH4ChatPlayerController::SetLobbyInput_Implementation()
{
    bShowMouseCursor = true;

    FInputModeGameAndUI Mode;
    Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    Mode.SetHideCursorDuringCapture(false);

    SetInputMode(Mode);
}

// 마우스 포인터 인게임 전용
void ACH4ChatPlayerController::SetInGameInput_Implementation()
{
    bShowMouseCursor = false;

    FInputModeGameOnly Mode;
    SetInputMode(Mode);
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