#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "PlayerState/CH4PlayerState.h"
#include "GameState/CH4GameStateBase.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "OutGameUI/CH4ChatUserWidget.h"
#include "OutGameUI/CH4ChatResultWidget.h"
#include "GameInstance/CH4GameInstance.h"
#include "Type/MatchTypes.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "Components/GridPanel.h"
#include "Components/Button.h"
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
    UE_LOG(LogTemp, Warning, TEXT("[PC] BeginPlay: %s  IsLobby=%d  Returning=%d"),
        *LevelName, bIsLobby ? 1 : 0, bReturningToLobby ? 1 : 0);

    if (bIsLobby)
    {
        bShowMouseCursor = true;
        {
            FInputModeGameAndUI Mode;
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Mode.SetHideCursorDuringCapture(false);
            SetInputMode(Mode);
        }
        if (bReturningToLobby)
        {
            UE_LOG(LogTemp, Warning, TEXT("[PC] ReturningToLobby==true → skip result check/request"));
        }
        else
        {
            if (UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>())
            {
                UE_LOG(LogTemp, Warning, TEXT("[PC] GI on Lobby: LastMatchState=%d FinalWinner=%d Roles=%d"),
                    (int32)GI->LastMatchState, (int32)GI->FinalWinner, GI->LastRoles.Num());

                if (GI->LastMatchState == EMatchTypes::GameOver &&
                    GI->FinalWinner != EWinTeam::None &&
                    PlayerState)
                {
                    bool bIsWin = false;

                    const EWinTeam WinTeam = GI->FinalWinner;
                    const FString  Key = PlayerState->GetPlayerName();

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
                        UE_LOG(LogTemp, Warning, TEXT("[PC] (GI OK) Role=%d -> MyTeam=%d  Win=%d"),
                            (int32)(*FoundRole), (int32)MyTeam, bIsWin ? 1 : 0);
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("[PC] (GI OK) Role NOT found for '%s'"), *Key);
                    }

                    ShowResultScreenLocal(bIsWin);
                    return;
                }
                UE_LOG(LogTemp, Warning, TEXT("[PC] GI not ready on Lobby -> Server_RequestMatchResult()"));
                Server_RequestMatchResult();
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("[PC] GameInstance is NULL on Lobby"));
            }
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

                UE_LOG(LogTemp, Warning, TEXT("[PC] Lobby UI shown"));
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
        return;
    }

    // ==== 인게임 분기 ====
    UE_LOG(LogTemp, Warning, TEXT("[PC] In-Game BeginPlay"));
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly{});
    bReturningToLobby = false;
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
    ShowResultScreenLocal(bIsWin);
}

void ACH4ChatPlayerController::ShowResultScreenLocal(bool bIsWin)
{
    UE_LOG(LogTemp, Warning, TEXT("[PC] ShowResultScreenLocal ENTER (bIsWin=%d)"), bIsWin ? 1 : 0);

    if (!ResultScreen)
    {
        UE_LOG(LogTemp, Error, TEXT("[PC] ResultScreen class NOT set on controller"));
        return;
    }

    if (ResultUI)
    {
        if (UButton* OldBtn = Cast<UButton>(ResultUI->GetWidgetFromName(TEXT("ReturnLobby"))))
        {
            OldBtn->OnClicked.Clear();
        }
        ResultUI->RemoveFromParent();
        ResultUI = nullptr;
    }

    ResultUI = CreateWidget<UCH4ChatResultWidget>(this, ResultScreen);
    if (!ResultUI)
    {
        UE_LOG(LogTemp, Error, TEXT("[PC] CreateWidget(ResultScreen) FAILED"));
        return;
    }

    ResultUI->AddToViewport(1000);

    bShowMouseCursor = true;
    {
        FInputModeGameAndUI Mode;
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetWidgetToFocus(ResultUI->TakeWidget());
        SetInputMode(Mode);
    }
    if (UImage* ResultImage = Cast<UImage>(ResultUI->GetWidgetFromName(TEXT("ResultScreen"))))
    {
        UTexture2D* WinTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Win.Win"));
        UTexture2D* LoseTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Lose.Lose"));
        if (WinTex && LoseTex)
        {
            ResultImage->SetBrushFromTexture(bIsWin ? WinTex : LoseTex);
            UE_LOG(LogTemp, Warning, TEXT("[PC] Result texture applied: %s"), bIsWin ? TEXT("WIN") : TEXT("LOSE"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[PC] Result textures NOT found"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PC] Image widget named 'ResultScreen' NOT found"));
    }

    if (UButton* ReturnBtn = Cast<UButton>(ResultUI->GetWidgetFromName(TEXT("ReturnLobby"))))
    {
        ReturnBtn->OnClicked.Clear();
        ReturnBtn->OnClicked.AddDynamic(this, &ACH4ChatPlayerController::OnResultReturnToLobbyClicked);
        UE_LOG(LogTemp, Warning, TEXT("[PC] Result Return button bound"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[PC] 'ReturnLobby' not found in ResultUI"));
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

void ACH4ChatPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!IsLocalController())
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
    const bool bIsLobby = LevelName.Contains(TEXT("LobbyMap"));
    if (bIsLobby)
    {
        bPrevMatchEnded = false;
        return;
    }

    AGameStateBase* GS = World->GetGameState();
    if (!GS)
    {
        return;
    }

    const bool bEnded = GS->HasMatchEnded();
    if (!bPrevMatchEnded && bEnded)
    {
        UE_LOG(LogTemp, Log, TEXT("[PC::Tick] Match ended detected. Waiting for server travel and GI values."));
        bCachedResult = true;
    }
    else if (bPrevMatchEnded && !bEnded)
    {
        bCachedResult = false;
    }

    bPrevMatchEnded = bEnded;
}

void ACH4ChatPlayerController::Server_RequestMatchResult_Implementation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("[PC][Server] GI nullptr"));
        return;
    }
    EWinTeam Winner = GI->FinalWinner;

    TArray<FPlayerRoleData> RoleArray;
    RoleArray.Reserve(GI->LastRoles.Num());
    for (const TPair<FString, EPlayerRole>& KVP : GI->LastRoles)
    {
        FPlayerRoleData D;
        D.PlayerName = KVP.Key;
        D.Role = KVP.Value;
        RoleArray.Add(D);
    }

    UE_LOG(LogTemp, Warning, TEXT("[PC][Server] Send Result: Winner=%d Roles=%d"), (int32)Winner, RoleArray.Num());
    Client_ReceiveMatchResult(Winner, RoleArray);
}

void ACH4ChatPlayerController::Client_ReceiveMatchResult_Implementation(EWinTeam Winner, const TArray<FPlayerRoleData>& Roles)
{
    if (bReturningToLobby)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC][Client] ignore result (returning to lobby)")); return;
    }
    UE_LOG(LogTemp, Warning, TEXT("[PC][Client] Receive Result: Winner=%d Roles=%d"), (int32)Winner, Roles.Num());

    if (Winner == EWinTeam::None || Roles.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC][Client] Result ignored (no valid winner or roles). Keep Lobby UI."));
        return;
    }

    UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>();
    if (!GI)
    {
        UE_LOG(LogTemp, Error, TEXT("[PC][Client] GI nullptr"));
        return;
    }
    GI->FinalWinner = Winner;
    GI->LastMatchState = EMatchTypes::GameOver;
    GI->LastRoles.Empty();
    for (const FPlayerRoleData& D : Roles)
    {
        GI->LastRoles.Add(D.PlayerName, D.Role);
    }
    bool bIsWin = false;
    if (PlayerState)
    {
        const FString Key = PlayerState->GetPlayerName();
        if (const EPlayerRole* FoundRole = GI->LastRoles.Find(Key))
        {
            EWinTeam MyTeam = EWinTeam::None;
            switch (*FoundRole)
            {
            case EPlayerRole::Police: MyTeam = EWinTeam::Police; break;
            case EPlayerRole::Thief:  MyTeam = EWinTeam::Thief;  break;
            default:                  MyTeam = EWinTeam::None;   break;
            }
            bIsWin = (MyTeam == Winner);
        }
    }
    ShowResultScreenLocal(bIsWin);
}

void ACH4ChatPlayerController::OnResultReturnToLobbyClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("[PC] Return-to-Lobby clicked"));

    if (bReturningToLobby)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PC] Already returning to lobby. Ignored."));
        return;
    }
    bReturningToLobby = true;
    Server_ConsumeMatchResult();
    if (UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>())
    {
        GI->LastMatchState = EMatchTypes::WaitingToStart;
        GI->FinalWinner = EWinTeam::None;
        GI->LastRoles.Empty();
        UE_LOG(LogTemp, Warning, TEXT("[PC] GI cleared (client)."));
    }
    {
        TArray<UUserWidget*> AllWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, AllWidgets, UUserWidget::StaticClass(), false);
        for (UUserWidget* W : AllWidgets)
        {
            if (!W) continue;
            const FString CName = W->GetClass() ? W->GetClass()->GetName() : TEXT("");
            const FString ObjName = W->GetName();
            if (CName.Contains(TEXT("Result")) || ObjName.Contains(TEXT("Result")))
            {
                W->RemoveFromParent();
                UE_LOG(LogTemp, Warning, TEXT("[PC] Removed widget: %s (%s)"), *ObjName, *CName);
            }
        }
    }
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly{});
    Server_RequestReturnLobby();
}

void ACH4ChatPlayerController::Server_ConsumeMatchResult_Implementation()
{
    if (UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>())
    {
        GI->LastMatchState = EMatchTypes::WaitingToStart;
        GI->FinalWinner = EWinTeam::None;
        GI->LastRoles.Empty();
        UE_LOG(LogTemp, Warning, TEXT("[PC][Server] GI cleared (consume result)."));
    }
}

// 게임 종료
void ACH4ChatPlayerController::ExitGame()
{
    if (UWorld* World = GetWorld())
    {
        UKismetSystemLibrary::QuitGame(World, this, EQuitPreference::Quit, false);
    }
}