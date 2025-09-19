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
    bReplicates = true;
    HeldItem = nullptr;
    bUsingItem = false;

    // �浹 �̺�Ʈ ���ε�
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AThiefCharacter::OnOverlapBegin);
}

// �浹 �̺�Ʈ
void AThiefCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // �̹� �������� ��� ������ ����
    if (HeldItem) return;

    // �浹�� ���� ������ ���Ͷ��
    if (OtherActor->ActorHasTag(TEXT("Item")))
    {
        UE_LOG(LogTemp, Log, TEXT("ĳ���Ͱ� �����۰� �浹�߽��ϴ�: %s"), *OtherActor->GetName());
        ServerPickupItem(OtherActor); // ������ ��û
    }
}

// �������� ������ �ݱ� ó��
void AThiefCharacter::ServerPickupItem_Implementation(AActor* ItemActor)
{
    if (!ItemActor) return;

    UBaseItem* Item = nullptr;
    UFunction* Func = ItemActor->FindFunction(FName("GetItemData"));
    if (Func)
    {
        struct FItemGetter { UBaseItem* ReturnValue; };
        FItemGetter ItemGetter;
        ItemActor->ProcessEvent(Func, &ItemGetter);
        Item = ItemGetter.ReturnValue;
    }

    if (Item)
    {
        PickupItem(Item);
        ItemActor->Destroy(); // �������� ������ ����
    }
}

// HeldItem ���� ���
void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThiefCharacter, HeldItem);
}

// HeldItem �� ���� �� UI ����
void AThiefCharacter::OnRep_HeldItem()
{
    UpdateHeldItemUI(HeldItem);
}

// ���� ������ �ݱ�
void AThiefCharacter::PickupItem(UBaseItem* Item)
{
    if (!Item || HeldItem) return;
    HeldItem = Item;
}

// ������ ��� �Է�
void AThiefCharacter::UseItemInput()
{
    if (HeldItem && !bUsingItem)
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
    if (HeldItem && !bUsingItem)
    {
        HandleUseItem(HeldItem);
    }
}

// ���� ������ ��� ó�� (���� Ŭ���� �ñ״�ó�� ����)
void AThiefCharacter::HandleUseItem(UBaseItem* ItemActor)
{
    if (!ItemActor) return;

    // �����ϰ� UBaseItem�� ĳ����
    UBaseItem* Item = Cast<UBaseItem>(ItemActor);
    if (!Item) return;

    bUsingItem = true;

    // ������ ���
    Item->UseItem(this);

    // �������Ʈ �̺�Ʈ ȣ��
    OnItemUsed(Item);

    // Ŭ���̾�Ʈ UI
    if (Item->IsA(UCokeItem::StaticClass()))
        ClientShowSpeedBoostUI();

    // ��� �� �ʱ�ȭ
    HeldItem = nullptr;

    // ���� ƽ�� bUsingItem ���� (���� ��� ��� �Լ�)
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AThiefCharacter::ResetUsingItem);
}

// bUsingItem �ʱ�ȭ
void AThiefCharacter::ResetUsingItem()
{
    bUsingItem = false;
}

// �������� ������ �� ���� ó��
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        Destroy();

        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            GS->OnRep_RemainingThieves();
        }

        ClientOnTrapped();
    }
}

// Ŭ���̾�Ʈ UI ó��
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed(nullptr);
}

void AThiefCharacter::ClientShowSpeedBoostUI()
{
    ShowSpeedBoostUI();
}
