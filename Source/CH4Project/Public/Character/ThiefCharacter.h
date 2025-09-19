#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"
#include "Item/BaseItem.h"
#include "Item/CokeItem.h"
#include "Item/TrapItem.h"
#include "Item/ClockItem.h"
#include "ThiefCharacter.generated.h"


UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACH4Character
{
    GENERATED_BODY()

public:
    // ������
    AThiefCharacter();

    //������ ��� �Է�, ���ÿ��� ȣ��
    virtual void UseItemInput() override;

    // ������ �ݱ�
    UFUNCTION(BlueprintCallable, Category = "Item")
    void PickupItem(UBaseItem* Item);

    // �������� ������ ��� ��û RPC
    virtual void ServerUseItem_Implementation() override;

    // �������� ������ �ݱ� ��û RPC
    UFUNCTION(Server, Reliable)
    void ServerPickupItem(AActor* ItemActor);
    void ServerPickupItem_Implementation(AActor* ItemActor);

    // �������� ������ �� ���������� ����
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();
    void ServerOnCaughtByPolice_Implementation();

    // HUD/UI Ŭ���̾�Ʈ��
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();
    void ClientOnTrapped_Implementation();

    // �ӵ� ���� UI Ŭ���̾�Ʈ��
    UFUNCTION(Client, Reliable)
    void ClientShowSpeedBoostUI();
    void ClientShowSpeedBoostUI_Implementation();

protected:
    // ĸ�� �浹 �̺�Ʈ
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    //���� ������ ��� ó�� (���������� ����)
    virtual void HandleUseItem(UBaseItem* Item);

    //��� ���� ������ �ʱ�ȭ �Լ�
    void ResetUsingItem();

    // ���� ������ �ִ� ������
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_HeldItem, Category = "Item")
    UBaseItem* HeldItem;

    // HeldItem ���� ����� �� �ڵ����� ȣ��Ǵ� �Լ�
    UFUNCTION()
    void OnRep_HeldItem();

    // ���� ���
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // �������Ʈ���� �����ϴ� ������ ��� ȿ�� �̺�Ʈ
    UFUNCTION(BlueprintImplementableEvent, Category = "Item|Effect")
    void OnItemUsed(UBaseItem* Item);

    // ������ UI: �ӵ� ���� ǥ�� (�������Ʈ���� ����)
    UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
    void ShowSpeedBoostUI();

    // UI ������Ʈ �������Ʈ �̺�Ʈ
    UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
    void UpdateHeldItemUI(UBaseItem* NewItem);
};