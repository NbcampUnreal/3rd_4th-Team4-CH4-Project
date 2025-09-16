#include "Item/TrapItem.h"
#include "Character/CH4Character.h"
#include "Engine/World.h"

void UTrapItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character || !TrapClass)
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Character;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    Character->GetWorld()->SpawnActor<AActor>(TrapClass, Character->GetActorLocation(), Character->GetActorRotation(), SpawnParams);
}