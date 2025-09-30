#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "PlayerState/CH4PlayerState.h"
#include "GameState/CH4GameStateBase.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "OutGameUI/CH4ChatUserWidget.h"
#include "GameInstance/CH4GameInstance.h"
#include "Type/MatchTypes.h"
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
    PrimaryActorTick.bCanEverTick = true;
}

void ACH4ChatPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) return;

    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    const bool bIsLobby = LevelName.Contains(TEXT("LobbyMap"));
    UE_LOG(LogTemp, Warning, TEXT("[PC] BeginPlay: Level=%s bIsLobby=%d"), *LevelName, bIsLobby ? 1 : 0);

    // ShowResultScreen(false);
    // return;

    if (bIsLobby)
    {
        bShowMouseCursor = true;
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        SetInputMode(Mode);

        if (UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>())
        {
            UE_LOG(LogTemp, Warning, TEXT("[PC] GI on Lobby: LastMatchState=%d  FinalWinner=%d  Roles.Num=%d"),
                (int32)GI->LastMatchState, (int32)GI->FinalWinner, GI->LastRoles.Num());

            if (GI->LastMatchState == EMatchTypes::GameOver)
            {
                bool bIsWin = false;

                const EWinTeam WinTeam = GI->FinalWinner;
                UE_LOG(LogTemp, Warning, TEXT("[PC] GameOver detected. WinTeam=%d"), (int32)WinTeam);

                if (WinTeam != EWinTeam::None && PlayerState)
                {
                    const FString Key = PlayerState->GetPlayerName();
                    UE_LOG(LogTemp, Warning, TEXT("[PC] MyKey=%s (lookup in LastRoles)"), *Key);

                    if (const EPlayerRole* FoundRole = GI->LastRoles.Find(Key))
                    {
                        EWinTeam MyTeam = EWinTeam::None;
                        switch (*FoundRole)
                        {
                        case EPlayerRole::Police: MyTeam = EWinTeam::Police; break;
                        case EPlayerRole::Thief:  MyTeam = EWinTeam::Thief;  break;
                        default:                  MyTeam = EWinTeam::None;   break;
                        }
                        bIsWin = (MyTeam == WinTeam);
                        UE_LOG(LogTemp, Warning, TEXT("[PC] Role found: %d -> MyTeam=%d  bIsWin=%d"),
                            (int32)(*FoundRole), (int32)MyTeam, bIsWin ? 1 : 0);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("[PC] Role NOT found for key '%s' (Roles.Num=%d). Show UI with bIsWin=%d"),
                            *Key, GI->LastRoles.Num(), bIsWin ? 1 : 0);
                    }
                }
                else
                {
                    if (WinTeam == EWinTeam::None)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[PC] WinTeam==None. Show UI with bIsWin=%d"), bIsWin ? 1 : 0);
                    }
                    else if (!PlayerState)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[PC] PlayerState NULL. Show UI with bIsWin=%d"), bIsWin ? 1 : 0);
                    }
                }

                UE_LOG(LogTemp, Warning, TEXT("[PC] ShowResultScreen(bIsWin=%d) -> call"), bIsWin ? 1 : 0);
                ShowResultScreen(bIsWin);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[PC] LastMatchState != GameOver -> build Lobby UI"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[PC] GameInstance NOT found on Lobby"));
        }

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
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[PC] CreateWidget(LobbyWidgetClass) FAILED"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[PC] LobbyWidgetClass NOT set on controller"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] In-Game BeginPlay"));
        bShowMouseCursor = false;
        FInputModeGameOnly Mode;
        SetInputMode(Mode);
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
    // 로비 UI 찾기
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UCH4ChatUserWidget::StaticClass(), false);

    for (UUserWidget* Widget : FoundWidgets)
    {
        if (UGridPanel* Grid = Cast<UGridPanel>(Widget->GetWidgetFromName(TEXT("PlayerList"))))
        {
            Grid->ClearChildren();

            if (AGameStateBase* GS = GetWorld()->GetGameState())
            {
                int32 Index = 0;
                for (APlayerState* PS : GS->PlayerArray)
                {
                    if (ACH4ChatPlayerState* MyPS = Cast<ACH4ChatPlayerState>(PS))
                    {
                        MyPS->OnPlayerNameUpdated.AddUniqueDynamic(
                            this, &ACH4ChatPlayerController::HandleAnyPlayerNameUpdated);

                        MyPS->OnReadyStateChanged.AddUniqueDynamic(
                            this, &ACH4ChatPlayerController::HandleAnyReadyChanged);
                    }

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
                                if (ACH4ChatPlayerState* MyPS2 = Cast<ACH4ChatPlayerState>(PS))
                                {
                                    ReadyBox->SetIsChecked(MyPS2->IsReady());
                                }
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

void ACH4ChatPlayerController::HandleAnyPlayerNameUpdated()
{
    RefreshPlayerList();
}

void ACH4ChatPlayerController::HandleAnyReadyChanged(bool)
{
    RefreshPlayerList();
}

void ACH4ChatPlayerController::ShowResultScreen_Implementation(bool bIsWin)
{
    UE_LOG(LogTemp, Warning, TEXT("[PC] ShowResultScreen ENTER (bIsWin=%d)"), bIsWin ? 1 : 0);

    if (!ResultScreen)
    {
        UE_LOG(LogTemp, Error, TEXT("[PC] ResultScreen class NOT set on controller"));
        return;
    }

    if (UUserWidget* ResultUI = CreateWidget<UUserWidget>(this, ResultScreen))
    {
        ResultUI->AddToViewport(1000);

        if (UImage* ResultImage = Cast<UImage>(ResultUI->GetWidgetFromName(TEXT("ResultScreen"))))
        {
            UTexture2D* WinTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Win.Win"));
            UTexture2D* LoseTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Lose.Lose"));

            if (!WinTex || !LoseTex)
            {
                UE_LOG(LogTemp, Error, TEXT("[PC] Result textures NOT found: Win=%p Lose=%p"), WinTex, LoseTex);
            }
            else
            {
                ResultImage->SetBrushFromTexture(bIsWin ? WinTex : LoseTex);
                UE_LOG(LogTemp, Warning, TEXT("[PC] Result texture applied: %s"), bIsWin ? TEXT("WIN") : TEXT("LOSE"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[PC] Image widget named 'ResultScreen' NOT found in Result UI"));
        }

        bShowMouseCursor = true;
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetWidgetToFocus(ResultUI->TakeWidget());
        SetInputMode(Mode);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PC] CreateWidget(ResultScreen) FAILED"));
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

void ACH4ChatPlayerController::Server_RequestReturnLobby_Implementation()
{
    if (HasAuthority())
    {
        GetWorld()->ServerTravel(TEXT("/Game/Maps/LobbyMap?listen"));
    }
}

// 로비로 복귀
void ACH4ChatPlayerController::ReturnLobby()
{
    UGameplayStatics::OpenLevel(this, FName("LobbyMap"));
}

void ACH4ChatPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsLocalController()) return;

    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    const bool bIsLobby = LevelName.Contains(TEXT("LobbyMap"));
    if (bIsLobby || bCachedResult) return;

    AGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState() : nullptr;
    if (!GS) return;

    const bool bEnded = GS->HasMatchEnded();

    if (!bPrevMatchEnded && bEnded)
    {
        if (UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>())
        {
            GI->LastMatchState = EMatchTypes::GameOver;

            EWinTeam WinTeam = EWinTeam::None;
            GI->FinalWinner = WinTeam;

            if (APlayerState* PS = PlayerState)
            {
                EPlayerRole MyRole = EPlayerRole::Citizen;
                GI->LastRoles.FindOrAdd(PS->GetPlayerName()) = MyRole;
            }

            bCachedResult = true;
        }
    }

    bPrevMatchEnded = bEnded;
}

// 게임 종료
void ACH4ChatPlayerController::ExitGame()
{
    if (UWorld* World = GetWorld())
    {
        UKismetSystemLibrary::QuitGame(World, this, EQuitPreference::Quit, false);
    }
}