#include "Character/ThiefCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Item/BaseItem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AThiefCharacter::AThiefCharacter()
{
   
}

// 경찰에게 잡혔을 때 서버에서만 실행
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    // 게임 모드를 가져와서 캐릭터를 파괴
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        Destroy();

        // 게임 상태를 가져와서 남은 도둑 수를 줄임
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            // OnRep 함수를 수동으로 호출하여 남은 도둑 수 변경을 모든 클라이언트에 알려줌
            GS->OnRep_RemainingThieves();
        }

        // 클라이언트에게 잡혔다는 UI를 보여주도록 요청
        ClientOnTrapped();
    }
}

// 클라이언트 UI 처리
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed(nullptr);
}
