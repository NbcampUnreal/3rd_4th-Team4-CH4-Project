#include "Item/TrapItem.h"
#include "Character/CH4Character.h"
#include "Components/CapsuleComponent.h"
#include "Item/TrapActor.h"
#include "Engine/World.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"

void UTrapItem::UseItem_Implementation(ACH4Character* Character)
{
    if (!Character || !TrapClass) return;
    if (!Character->HasAuthority()) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Character;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 캐릭터 발밑에서 아래로 라인트레이스
    FVector Start = Character->GetActorLocation();
    FVector End = Start - FVector(0.f, 0.f, 500.f); // 500만큼 아래로 탐색

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Character); // 자기 자신은 무시

    FVector SpawnLocation = Start; // 기본값: 캐릭터 위치
    FRotator SpawnRotation = Character->GetActorRotation();

    if (Character->GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECC_Visibility,
            Params))
    {
        SpawnLocation = HitResult.Location;          // 충돌한 지점(바닥)

        SpawnLocation.Z += 45.f;
    }

    // 트랩 소환
    Character->GetWorld()->SpawnActor<ATrapActor>(
        TrapClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );
}
