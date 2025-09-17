#include "Character/ThiefCharacter.h"
#include "Character/CH4Character.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"

AThiefCharacter::AThiefCharacter()
    : Super() // �θ� ������ ȣ��
{
    bReplicates = true;
    HeldItem = nullptr;
    bUsingItem = false;
}

void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThiefCharacter, HeldItem);
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
    if (HeldItem)
    {
        HandleUseItem(HeldItem);
    }
}

void AThiefCharacter::HandleUseItem(AActor* ItemActor)
{
    if (!ItemActor) return;

    bUsingItem = true;

    if (ItemActor->ActorHasTag("SpeedBoost"))
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

        // �����ڿ��Ը� HUD ���̰� �ϴ� Ŭ���̾�Ʈ RPC ȣ�� (���� ȣ��)
        ClientShowSpeedBoostUI();
    }
    else if (ItemActor->ActorHasTag("Trap"))
    {
        if (TrapClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // �������� Ʈ�� ����
            GetWorld()->SpawnActor<AActor>(TrapClass, GetActorLocation(), GetActorRotation(), SpawnParams);

            // Ŭ�� ��ü�� ȿ��
            MulticastUseTrap();
        }
    }
    else if (ItemActor->ActorHasTag("Clock"))
    {
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            // ���� �ð� 5�� ���̱� (���� ����)
            GS->MatchTime = FMath::Max(0.f, GS->MatchTime - 5.f);

            GS->OnRep_MatchTime(); // Ŭ�� HUD ���� Ʈ����
        }

        // Ŭ�� ��ü ����Ʈ
        MulticastUseClock();
    }

    // ������ ����
    ItemActor->Destroy();
    HeldItem = nullptr;

    //������ ȿ�� ����(�ȳ����� ������ ����) ����Ʈ ����� ���� �����̸� �־��µ� �ʿ� ������ bUsingItem = false; �� ����� �� 
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            bUsingItem = false;
        });
}

void AThiefCharacter::MulticastUseSpeedBoost_Implementation()
{
    // ��� Ŭ�󿡼� ������ ����Ʈ/����
    OnSpeedBoostEffect();
    UE_LOG(LogTemp, Log, TEXT("Speed Boost Effect"));
}

void AThiefCharacter::MulticastUseTrap_Implementation()
{
    // �� ��ġ ȿ��
    OnTrapEffect();
    UE_LOG(LogTemp, Log, TEXT("Trap Placed"));
}

void AThiefCharacter::MulticastUseClock_Implementation()
{
    // �ð� ȿ��
    OnClockEffect();
    UE_LOG(LogTemp, Log, TEXT("Clock Used Time Reduced."));
}

// �������� ������ ��
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    // GameMode ��������
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        // ���� ���� ó��
        Destroy(); // Ȥ�� ���� ��ȯ

        // GameState ��������
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            GS->OnRep_RemainingThieves(); // �������� �ٷ� UI ���� Ʈ����
        }

        // Ŭ�� ���� ���� UI ����
        ClientOnTrapped();
    }
}

// Trap�� �ɷ��� �� UI ǥ��
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnTrapEffect();
}

// �������Ʈ���� ������ �� �ֵ��� ����
UFUNCTION(BlueprintImplementableEvent, Category = "UI")
void OnTrappedUI();

// �̼� ������ ��� UI ǥ��
void AThiefCharacter::ClientShowSpeedBoostUI_Implementation()
{
    ShowSpeedBoostUI();
}

// �������Ʈ���� ������ �� �ֵ��� ����
UFUNCTION(BlueprintImplementableEvent, Category = "UI")
void ShowSpeedBoostUI();
