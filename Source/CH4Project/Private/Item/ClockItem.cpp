#include "Item/ClockItem.h"
#include "Character/CH4Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameState/CH4GameStateBase.h"
#include "GameMode/CH4GameMode.h"
#include "Kismet/GameplayStatics.h"

void UClockItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character)
    {
        return;
    }

    if (AGameStateBase* GS = UGameplayStatics::GetGameState(Character))
    {
        if (ACH4GameStateBase* CH4GS = Cast<ACH4GameStateBase>(GS))
        {
            CH4GS->MatchTime = FMath::Max(0.f, CH4GS->MatchTime - TimeToReduce);
        }
    }
}