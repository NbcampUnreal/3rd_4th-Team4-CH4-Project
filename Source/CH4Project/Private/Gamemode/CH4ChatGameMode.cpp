#include "Gamemode/CH4ChatGameMode.h"
#include "GameState/CH4ChatGameState.h"
#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Kismet/GameplayStatics.h"

// ���� ��� �Ҵ� �� �⺻������ ž��Ǵ� Ŭ����
ACH4ChatGameMode::ACH4ChatGameMode()
{
	PlayerControllerClass = ACH4ChatPlayerController::StaticClass();
	PlayerStateClass = ACH4ChatPlayerState::StaticClass();
	GameStateClass = ACH4ChatGameState::StaticClass();
}
// �κ� �� ���� �� UI�۵� -> ��Ʈ�ѷ����� �ٽ� �����
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