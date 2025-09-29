#include "Item/CokeItem.h"
#include "Character/CH4Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UCokeItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character || !Character->GetCharacterMovement()) return;
    if (!Character->HasAuthority()) return;

    UWorld* World = Character->GetWorld();
    if (!World) return;

    // 이미 버프가 적용 중인지 확인
    bool bIsBuffActive = World->GetTimerManager().IsTimerActive(TimerHandle_ResetSpeed);

    if (!bIsBuffActive)
    {
        // 처음 사용할 때만 기본 속도 기준으로 +300 적용
       Character->GetCharacterMovement()->MaxWalkSpeed = Character->WalkSpeed + 300.f;
    }

    // 항상 리셋 타이머만 새로 갱신
    World->GetTimerManager().SetTimer(
        TimerHandle_ResetSpeed,
        FTimerDelegate::CreateLambda([Character]()
        {
            if (Character && Character->GetCharacterMovement())
            {
                Character->GetCharacterMovement()->MaxWalkSpeed = Character->WalkSpeed;
            }
        }),
        5.0f,
        false
    );
}