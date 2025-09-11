#include "Character/ThiefCharacter.h"
#include "Character/CH4Character.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"

AThiefCharacter::AThiefCharacter()
    : Super() // �θ� ������ ȣ��
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
        // ���� �ӵ� ����
        float OriginalSpeed = GetCharacterMovement()->MaxWalkSpeed;

        // �ӵ� ����
        GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed + 300.f;

        // ���� �ð� �� ���� �ӵ��� �ǵ�����
        FTimerHandle TimerHandle_ResetSpeed;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_ResetSpeed,
            FTimerDelegate::CreateLambda([this, OriginalSpeed]()
                {
                    GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
                }),
            5.0f, // ���� �ð� (�ӽ� 5��)
            false
        );
        // Ŭ�� ��ü ����Ʈ
        MulticastUseSpeedBoost();
    }
    else if (ItemActor->ActorHasTag("TrapItem"))
    {
        if (TrapClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // �������� Ʈ�� ����
            GetWorld()->SpawnActor<AActor>(TrapClass, GetActorLocation(), GetActorRotation(), SpawnParams);

            // Ŭ�� ��ü�� ȿ��
            MulticastUseTrap();
        }
    }
    else if (ItemActor->ActorHasTag("ClockItem"))
    {
        if (AGameStateBase* GS = GetWorld()->GetGameState())
        {
            // ���� �ð� ó��
        }

        MulticastUseClock();
    }

    // ������ ����
    ItemActor->Destroy();
    HeldItem = nullptr;

    //������ ��� ����(�ȳ����� ������ ����) ����Ʈ ����� ���� �����̸� �־��µ� �ʿ� ������ bUsingItem = false; �� ����� �� 
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            bUsingItem = false;
        });
}

void AThiefCharacter::MulticastUseSpeedBoost_Implementation()
{
    // ��� Ŭ�󿡼� ������ ����Ʈ/����
    UE_LOG(LogTemp, Log, TEXT("Speed Boost Effect"));
}

void AThiefCharacter::MulticastUseTrap_Implementation()
{
    // �� ��ġ ȿ��
    UE_LOG(LogTemp, Log, TEXT("Trap Placed"));
}

void AThiefCharacter::MulticastUseClock_Implementation()
{
    // �ð� ȿ��
    UE_LOG(LogTemp, Log, TEXT("Clock Used Time Reduced."));
}

void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThiefCharacter, HeldItem);
}
