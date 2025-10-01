#include "Item/ClockItem.h"
#include "Character/CH4Character.h"
#include "Character/ThiefCharacter.h"
#include "Character/PoliceCH4Character.h"
#include "GameFramework/GameStateBase.h"
#include "GameState/CH4GameStateBase.h"
#include "Kismet/GameplayStatics.h"

void UClockItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character) return;

    UE_LOG (LogTemp, Warning, TEXT("Working"))
    if (AGameStateBase* GS = UGameplayStatics::GetGameState(Character))
    {
        if (ACH4GameStateBase* CH4GS = Cast<ACH4GameStateBase>(GS))
        {
            if (Cast<AThiefCharacter>(Character))
            {
                // 도둑 → 시간 감소
                CH4GS->MatchTime = FMath::Max(0.f, CH4GS->MatchTime - TimeToReduce);
            }
            else if (Cast<APoliceCH4Character>(Character))
            {
                // 경찰 → 시간 증가
                CH4GS->MatchTime += TimeToReduce;
            }
        }
    }
}