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
    void PickupItem(AActor* ItemActor);

    // ������ ��� �Է�
    UFUNCTION()
    void UseItemInput();

    // �������� ������ ��� ó��
    UFUNCTION(Server, Reliable)
    void ServerUseItem();

protected:
    // ���� ������ ��� ó�� (���������� ����)
    void HandleUseItem(AActor* ItemActor);

    // ������ ��� ȿ���� ��� Ŭ���̾�Ʈ�� �˸���
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseSpeedBoost();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseTrap();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseClock();

    // ���� ������ �ִ� ������
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    AActor* HeldItem;

    // Trap Ŭ���� (�������Ʈ���� ����)
    UPROPERTY(EditDefaultsOnly, Category = "Item")
    TSubclassOf<AActor> TrapClass;

    // ������ ������ ���
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
