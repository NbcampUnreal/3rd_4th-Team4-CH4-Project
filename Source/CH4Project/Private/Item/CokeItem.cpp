#include "Item/CokeItem.h"
#include "Character/CH4Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UCokeItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character || !Character->GetCharacterMovement()) return;
    if (!Character->HasAuthority()) return;

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    UWorld* World = Character->GetWorld();
    if (!World) return;

    // 원래 속도를 캐싱 (한번만)
    static float OriginalSpeed = 0.f;
    if (OriginalSpeed <= 0.f)
    {
        OriginalSpeed = Movement->MaxWalkSpeed;
    }

    // 타이머 핸들은 멤버로 두는 게 좋음 (아이템이 사라져도 관리 가능)
    if (!TimerHandle_ResetSpeed.IsValid())
    {
        // 버프 적용 (처음 사용할 때만 속도 +300)
        Movement->MaxWalkSpeed = OriginalSpeed + 300.f;
    }

    // 항상 타이머만 새로 갱신
    World->GetTimerManager().SetTimer(
        TimerHandle_ResetSpeed,
        FTimerDelegate::CreateLambda([Character]()
        {
            if (Character && Character->GetCharacterMovement())
            {
                Character->GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
            }
        }),
        5.0f,
        false
    );
}