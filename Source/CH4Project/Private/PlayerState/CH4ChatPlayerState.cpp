#include "PlayerState/CH4ChatPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Gamemode/CH4ChatGameMode.h"

ACH4ChatPlayerState::ACH4ChatPlayerState()
{
    bReplicates = true;
    bIsReady = false;
}

void ACH4ChatPlayerState::OnRep_IsReady()
{

}

bool ACH4ChatPlayerState::ServerSetReady_Validate(bool bNewReady) { return true; }

void ACH4ChatPlayerState::ServerSetReady_Implementation(bool bNewReady)
{
    if (bIsReady != bNewReady)
    {
        bIsReady = bNewReady;

        if (UWorld* World = GetWorld())
        {
            if (ACH4ChatGameMode* GM = World->GetAuthGameMode<ACH4ChatGameMode>())
            {
                GM->CheckAllPlayersReady();
            }
        }
    }
}

void ACH4ChatPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACH4ChatPlayerState, bIsReady);
}