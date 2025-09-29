#include "GameState/CH4ChatGameState.h"
#include "GameFramework/PlayerController.h"
#include "PlayerController/CH4ChatPlayerController.h"

void ACH4ChatGameState::MulticastRefreshPlayerList_Implementation()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (ACH4ChatPlayerController* PC = Cast<ACH4ChatPlayerController>(It->Get()))
        {
            PC->RefreshPlayerList();
        }
    }
}

void ACH4ChatGameState::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);
    MulticastRefreshPlayerList();
}

void ACH4ChatGameState::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);
    MulticastRefreshPlayerList();
}