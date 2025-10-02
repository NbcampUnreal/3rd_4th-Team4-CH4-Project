#include "Gamemode/CH4ChatGameMode.h"
#include "GameState/CH4ChatGameState.h"
#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameInstance/CH4GameInstance.h"

// 게임 모드 할당 시 기본적으로 탑재되는 클래스
ACH4ChatGameMode::ACH4ChatGameMode()
{
	PlayerControllerClass = ACH4ChatPlayerController::StaticClass();
	PlayerStateClass = ACH4ChatPlayerState::StaticClass();
	GameStateClass = ACH4ChatGameState::StaticClass();
}

// 로비 맵 실행 시 UI작동 -> 컨트롤러에서 다시 만들기
void ACH4ChatGameMode::BeginPlay()
{
    Super::BeginPlay();

}

void ACH4ChatGameMode::CheckAllPlayersReady()
{
    int32 Total = 0;
    int32 Ready = 0;

    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC) continue;

            if (ACH4ChatPlayerState* P = Cast<ACH4ChatPlayerState>(PC->PlayerState))
            {
                ++Total;
                if (P->IsReady())
                {
                    ++Ready;
                }
            }
        }
    }

    if (Total >= 2 && Total == Ready)
    {
        UE_LOG(LogTemp, Log, TEXT("[Server] All players ready. Starting game..."));
        StartGame();
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("[Server] Players Ready %d/%d (Waiting...)"), Ready, Total);
    }

    if (ACH4ChatGameState* GS = GetGameState<ACH4ChatGameState>())
    {
        GS->MulticastRefreshPlayerList();
    }
}

void ACH4ChatGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (NewPlayer && NewPlayer->PlayerState)
    {
        APlayerState* PS = NewPlayer->PlayerState;
        const FString CurName = PS->GetPlayerName();
        const bool bNeedName =
            CurName.IsEmpty() ||
            CurName.Equals(TEXT("Player"), ESearchCase::IgnoreCase) ||
            CurName.Equals(TEXT("PlayerName"), ESearchCase::IgnoreCase);

        if (bNeedName)
        {
            const int32 PlayerIndex = GameState.Get() ? GameState->PlayerArray.Num() : 0;
            const FString NewName = FString::Printf(TEXT("Player_%d"), PlayerIndex);
            PS->SetPlayerName(NewName);
            UE_LOG(LogTemp, Log, TEXT("[Server][Lobby] Name set: %s"), *NewName);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("[Server][Lobby] Keep existing name: %s"), *CurName);
        }
    }

    int32 Total = 0, Ready = 0;
    if (UWorld* World = GetWorld())
    {
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* PC = It->Get();
            if (!PC) continue;

            if (ACH4ChatPlayerState* P = Cast<ACH4ChatPlayerState>(PC->PlayerState))
            {
                ++Total;
                if (P->IsReady()) ++Ready;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[Server] Players now: %d (Ready %d/%d)"), Total, Ready, Total);

    if (ACH4ChatGameState* GS = GetGameState<ACH4ChatGameState>())
    {
        GS->MulticastRefreshPlayerList();
    }
}

void ACH4ChatGameMode::Logout(AController* Exiting)
{
    // 나가기 전 정보 로깅
    FString PlayerName = TEXT("Unknown");
    bool bWasReady = false;
    if (Exiting && Exiting->PlayerState)
    {
        PlayerName = Exiting->PlayerState->GetPlayerName();
        if (ACH4ChatPlayerState* MyPS = Cast<ACH4ChatPlayerState>(Exiting->PlayerState))
        {
            bWasReady = MyPS->IsReady();
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[Server] %s has left the game (Ready=%s)"),
        *PlayerName, bWasReady ? TEXT("true") : TEXT("false"));

    Super::Logout(Exiting);

    // 제거 반영된 GameState 기준으로 현재 인원, Ready 집계
    int32 Total = -1;
    int32 Ready = 0;

    if (AGameStateBase* GSBase = GetGameState<AGameStateBase>())
    {
        Total = GSBase->PlayerArray.Num();

        for (APlayerState* PS : GSBase->PlayerArray)
        {
            if (const ACH4ChatPlayerState* P = Cast<ACH4ChatPlayerState>(PS))
            {
                if (P->IsReady()) ++Ready;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[Server] Players remaining: %d (Ready %d/%d)"),
        Total, Ready, Total);

    // 모든 클라이언트에 로비 리스트 갱신
    if (ACH4ChatGameState* GS = GetGameState<ACH4ChatGameState>())
    {
        GS->MulticastRefreshPlayerList();
    }
}

void ACH4ChatGameMode::StartGame()
{
    if (UWorld* World = GetWorld())
    {
        // 인게임 입력 모드로 전환
        for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
        {
            if (ACH4ChatPlayerController* PC = Cast<ACH4ChatPlayerController>(It->Get()))
            {
                PC->SetInGameInput();
            }
        }

        // 맵 이동
        World->ServerTravel(TEXT("InGameMap"));
    }
}

void ACH4ChatGameMode::EndMatchAndShowResult()
{
    if (!HasAuthority()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>();
    const EWinTeam WinTeam = GI ? GI->FinalWinner : EWinTeam::None;

    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        ACH4ChatPlayerController* PC = Cast<ACH4ChatPlayerController>(It->Get());
        if (!PC) continue;

        APlayerState* PS = PC->PlayerState;
        bool bIsWin = false;

        if (GI && PS && WinTeam != EWinTeam::None)
        {
            const FString Key = PS->GetPlayerName();

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
            }
        }
        PC->ShowResultScreen(bIsWin);
    }
}