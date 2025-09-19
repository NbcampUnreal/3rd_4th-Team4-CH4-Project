#include "Gamemode/CH4ChatGameMode.h"
#include "GameState/CH4ChatGameState.h"
#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Kismet/GameplayStatics.h"

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
    if (!GameState) return;

    bool bAllReady = true;
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (const auto MyPS = Cast<ACH4ChatPlayerState>(PS))
        {
            if (!MyPS->IsReady()) { bAllReady = false; break; }
        }
    }

    if (bAllReady)
    {
        StartGame();
    }
}

void ACH4ChatGameMode::StartGame()
{
    UWorld* World = GetWorld();
    if (!World) return;

    World->ServerTravel(TEXT("/Game/Maps/InGameMap_Prototype?listen"));
}