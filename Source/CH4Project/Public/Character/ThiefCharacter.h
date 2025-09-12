#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"   // �θ� Ŭ���� ���
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACH4Character
{
    GENERATED_BODY()

public:
    AThiefCharacter();

    // ������ �ݱ�
    UFUNCTION(BlueprintCallable)
    void PickupItem(AActor* ItemActor);

    // ������ ��� �Է�
    UFUNCTION(BlueprintCallable)
    void UseItemInput();

    // �������� ������ ��� ó��
    UFUNCTION(Server, Reliable)
    void ServerUseItem();

    // �������� ������ �� (���������� ����)
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();

    // �ð� ������ ���� (��� �÷��̾� HUD ����)
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseClock();

    UFUNCTION(BlueprintImplementableEvent)
    void OnClockEffect();

    // Trap�� �ɷ��� �� UI ǥ��
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();

    UFUNCTION(BlueprintImplementableEvent)
    void OnTrapEffect();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseTrap();

    // �̼� ������ ���� UI ǥ��
    UFUNCTION(Client, Reliable)
    void ClientShowSpeedBoostUI();
protected:
    // ���� ������ ��� ó�� (���������� ����)
    void HandleUseItem(AActor* ItemActor);

    // ������ ��� ȿ���� ��� Ŭ���̾�Ʈ�� �˸���
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseSpeedBoost();

    UFUNCTION(BlueprintImplementableEvent)
    void OnSpeedBoostEffect();


    // ���� ������ �ִ� ������
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    AActor* HeldItem;

    // Trap Ŭ���� (�������Ʈ���� ����)
    UPROPERTY(EditDefaultsOnly, Category = "Item")
    TSubclassOf<AActor> TrapClass;

    // ������ ������ ���
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
