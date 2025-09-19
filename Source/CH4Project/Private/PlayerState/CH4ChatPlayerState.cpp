#include "PlayerState/CH4ChatPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Gamemode/CH4ChatGameMode.h"

ACH4ChatPlayerState::ACH4ChatPlayerState()
{
    bReplicates = true;
    bIsReady = false;
}

bool ACH4ChatPlayerState::ServerSetReady_Validate(bool bNewReady) { return true; }

void ACH4ChatPlayerState::ServerSetReady_Implementation(bool bNewReady)
{
    if (bIsReady != bNewReady)
    {
        bIsReady = bNewReady;

        // 값이 바뀌면 서버에서 즉시 전체 검사
        if (UWorld* World = GetWorld())
        {
            if (ACH4ChatGameMode* GM = World->GetAuthGameMode<ACH4ChatGameMode>())
            {
                GM->CheckAllPlayersReady(); // GameMode 쪽에 이미 구현됨 
            }
        }
    }
}

void ACH4ChatPlayerState::OnRep_IsReady() {}

void ACH4ChatPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACH4ChatPlayerState, bIsReady);
}