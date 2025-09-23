#include "Item/CokeItem.h"
#include "Character/CH4Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UCokeItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character || !Character->GetCharacterMovement())
    {
        return;
    }
    
    if (!Character->HasAuthority()) return;

    float OriginalSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
    Character->GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed + 300.f;

    FTimerHandle TimerHandle_ResetSpeed;
    if (UWorld* World = Character->GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TimerHandle_ResetSpeed,
            FTimerDelegate::CreateLambda([Character, OriginalSpeed]()
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
}