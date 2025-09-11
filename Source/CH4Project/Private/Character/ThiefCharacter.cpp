#include "Character/ThiefCharacter.h"
#include "Character/CH4Character.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"

AThiefCharacter::AThiefCharacter()
    : Super() // 부모 생성자 호출
{
    bReplicates = true;
    HeldItem = nullptr;
    bUsingItem = false;
}

void AThiefCharacter::PickupItem(AActor* ItemActor)
{
    if (ItemActor)
    {
        HeldItem = ItemActor;
    }
}

void AThiefCharacter::UseItemInput()
{
    if (HeldItem)
    {
        if (HasAuthority())
        {
            HandleUseItem(HeldItem);
        }
        else
        {
            ServerUseItem();
        }
    }
}

void AThiefCharacter::ServerUseItem_Implementation()
{
    HandleUseItem(HeldItem);
}

void AThiefCharacter::HandleUseItem(AActor* ItemActor)
{
    if (!ItemActor) return;

    bUsingItem = true;

    if (ItemActor->ActorHasTag("SpeedBoostItem"))
    {
        // 원래 속도 저장
        float OriginalSpeed = GetCharacterMovement()->MaxWalkSpeed;

        // 속도 증가
        GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed + 300.f;

        // 일정 시간 후 원래 속도로 되돌리기
        FTimerHandle TimerHandle_ResetSpeed;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_ResetSpeed,
            FTimerDelegate::CreateLambda([this, OriginalSpeed]()
                {
                    GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
                }),
            5.0f, // 지속 시간 (임시 5초)
            false
        );
        // 클라 전체 이펙트
        MulticastUseSpeedBoost();
    }
    else if (ItemActor->ActorHasTag("TrapItem"))
    {
        if (TrapClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // 서버에서 트랩 스폰
            GetWorld()->SpawnActor<AActor>(TrapClass, GetActorLocation(), GetActorRotation(), SpawnParams);

            // 클라 전체에 효과
            MulticastUseTrap();
        }
    }
    else if (ItemActor->ActorHasTag("ClockItem"))
    {
        if (AGameStateBase* GS = GetWorld()->GetGameState())
        {
            // 게임 시간 처리
        }

        MulticastUseClock();
    }

    // 아이템 제거
    ItemActor->Destroy();
    HeldItem = nullptr;

    //아이템 사용 해제(안넣으면 영원히 사용됨) 이팩트 재생을 위해 딜레이를 넣었는데 필요 없으면 bUsingItem = false; 만 남기면 됨 
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            bUsingItem = false;
        });
}

void AThiefCharacter::MulticastUseSpeedBoost_Implementation()
{
    // 모든 클라에서 보여줄 이펙트/사운드
    UE_LOG(LogTemp, Log, TEXT("Speed Boost Effect"));
}

void AThiefCharacter::MulticastUseTrap_Implementation()
{
    // 덫 설치 효과
    UE_LOG(LogTemp, Log, TEXT("Trap Placed"));
}

void AThiefCharacter::MulticastUseClock_Implementation()
{
    // 시계 효과
    UE_LOG(LogTemp, Log, TEXT("Clock Used Time Reduced."));
}

void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThiefCharacter, HeldItem);
}
