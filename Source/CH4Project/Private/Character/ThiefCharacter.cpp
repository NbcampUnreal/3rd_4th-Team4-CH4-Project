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
    // �浹�� ���Ͱ� ���ų�, �̹� �������� ������ ������ ����
    if (!OtherActor || HeldItem)
    {
        return;
    }

    // �浹�� ���Ϳ� "Item" �±װ� �ִ��� Ȯ��
    if (OtherActor->ActorHasTag(TEXT("Item")))
    {
        UE_LOG(LogTemp, Log, TEXT("Get Item: %s"), *OtherActor->GetName());
        // ������ �ݱ� ��û�� ������ ������
        ServerPickupItem(OtherActor);
    }
}

// �������� ������ �ݱ� ó��
void AThiefCharacter::ServerPickupItem_Implementation(AActor* ItemActor)
{
    // ��Ʈ��ũ�� ���޵� ItemActor�� ��ȿ���� �ٽ� �ѹ� ����
    if (!ItemActor || !ItemActor->IsValidLowLevel() || !ItemActor->ActorHasTag(TEXT("Item")))
    {
        return;
    }

    // ItemActor���� ������ �����͸� �������� �Լ��� ã�� ȣ��
    UBaseItem* Item = nullptr;
    UFunction* Func = ItemActor->FindFunction(FName("GetItemData"));
    if (Func)
    {
        struct FItemGetter { UBaseItem* ReturnValue; };
        FItemGetter ItemGetter;
        ItemActor->ProcessEvent(Func, &ItemGetter);
        Item = ItemGetter.ReturnValue;
    }

    // ������ �����Ͱ� ��ȿ�ϸ� ������ �������� �ݰ�, ���忡�� �ش� ���͸� �ı�
    if (Item)
    {
        PickupItem(Item);
        ItemActor->Destroy();
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
    // HeldItem ���� ����� ������ �������Ʈ�� UI�� ������Ʈ�ϵ��� ȣ��
    UpdateHeldItemUI(HeldItem);
}

// ���� ������ �ݱ�
void AThiefCharacter::PickupItem(UBaseItem* Item)
{
    // �̹� �������� �ְų�, �ݴ� �������� ��ȿ���� ������ ����
    if (!Item || HeldItem)
    {
        return;
    }
    HeldItem = Item;
}

// ������ ��� �Է�
void AThiefCharacter::UseItemInput()
{
    // �������� �ְ�, ���� ��� ���� ���°� �ƴ϶��
    if (HeldItem && !bUsingItem)
    {
        // ������� �ٷ� ��� ó��, �ƴ϶�� ������ ��� ��û
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

// �������� ������ ��� ��û�� �޾Ƽ� ó��
void AThiefCharacter::ServerUseItem_Implementation()
{
    if (HeldItem && !bUsingItem)
    {
        HandleUseItem(HeldItem);
    }
}

// ���� ������ ��� ó��
void AThiefCharacter::HandleUseItem(UBaseItem* Item)
{
    if (!Item)
    {
        return;
    }
    // ������ ��� ���·� ����
    bUsingItem = true;

    // �������� UseItem �Լ��� ȣ���Ͽ� ȿ���� �߻�
    Item->UseItem(this);

    // �������Ʈ �̺�Ʈ ȣ��
    OnItemUsed(Item);

    // Ư�� ������(�ݶ�)�̶�� �ӵ� ���� UI�� Ŭ���̾�Ʈ�� ǥ���ϵ��� ��û
    if (Item->IsA(UCokeItem::StaticClass()))
    {
        ClientShowSpeedBoostUI();
    }

    // ����� �������Ƿ� ������ �ʱ�ȭ
    HeldItem = nullptr;

    // ���� ƽ(������)�� bUsingItem ���¸� �ٽ� �ʱ�ȭ
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AThiefCharacter::ResetUsingItem);
}

// bUsingItem �ʱ�ȭ
void AThiefCharacter::ResetUsingItem()
{
    bUsingItem = false;
}

// �������� ������ �� ���������� ����
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    // ���� ��带 �����ͼ� ĳ���͸� �ı�
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        Destroy();

        // ���� ���¸� �����ͼ� ���� ���� ���� ����
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            // OnRep �Լ��� �������� ȣ���Ͽ� ���� ���� �� ������ ��� Ŭ���̾�Ʈ�� �˷���
            GS->OnRep_RemainingThieves();
        }

        // Ŭ���̾�Ʈ���� �����ٴ� UI�� �����ֵ��� ��û
        ClientOnTrapped();
    }
}

// Ŭ���̾�Ʈ UI ó��
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed(nullptr);
}

void AThiefCharacter::ClientShowSpeedBoostUI_Implementation()
{
    ShowSpeedBoostUI();
}