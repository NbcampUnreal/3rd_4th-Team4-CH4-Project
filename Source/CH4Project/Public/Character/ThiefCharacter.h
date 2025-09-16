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

    // ������ ��� �Է�
    virtual void UseItemInput() override;
    void ServerUseItem_Implementation();

    // ������ �ݱ�
    UFUNCTION(BlueprintCallable)
    void PickupItem(AActor* ItemActor);

    // �������� ������ �� (���������� ����)
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();

    // ������ ȿ���� ��� Ŭ���̾�Ʈ�� ����
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseClock();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseTrap();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseSpeedBoost();

    // HUD �� UI�� ���� Ŭ���̾�Ʈ ���� �Լ�
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();

    UFUNCTION(Client, Reliable)
    void ClientShowSpeedBoostUI();


protected:
    // ���� ������ ��� ó�� (���������� ����)
    void HandleUseItem(AActor* ItemActor);

    void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // ���� ������ �ִ� ������
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    AActor* HeldItem;

    // Trap Ŭ���� (�������Ʈ���� ����)
    UPROPERTY(EditDefaultsOnly, Category = "Item")
    TSubclassOf<AActor> TrapClass;

    // ������ ������ ���
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // �������Ʈ���� �����ϴ� �̺�Ʈ��
    UFUNCTION(BlueprintImplementableEvent)
    void OnClockEffect();

    UFUNCTION(BlueprintImplementableEvent)
    void OnTrapEffect();

    UFUNCTION(BlueprintImplementableEvent)
    void OnSpeedBoostEffect();

    UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
    void ShowSpeedBoostUI(); // ������ HUD���� ������ UI
    };
