#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"
#include "PoliceCH4Character.generated.h"

class UInputAction;
class AActor;

UCLASS()
class CH4PROJECT_API APoliceCH4Character : public ACH4Character
{
    GENERATED_BODY()

public:
    APoliceCH4Character();

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /* ================= Arrest (ü��) ================= */
    // ���� �Է� �� ���� ����
    UFUNCTION() void OnArrestInput();

    // ����: ���� Ž�� �� GameMode�� ����(����/�ù� �б�)
    UFUNCTION(Server, Reliable)
    void ServerTryArrest();

    // ���: ü�� ����(����/����)
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayArrestFX(bool bSuccess);

    // Ŭ�� ����: ü�� ��� UI (ü���� ���θ�)
    UFUNCTION(Client, Reliable)
    void ClientShowArrestResultUI(bool bSuccess);

    // ���� ��ü Ʈ���̽��� ���� ����� Pawn �ϳ�
    AActor* FindArrestTarget(float TraceDistance = 220.f, float Radius = 60.f) const;

    /* ================= Item Pickup ================= */
    // ������ ���Ϳ� ��ħ ���� �� ���� ���� �õ�
    UFUNCTION() void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    // ����: ������ ����, ���� ����
    UFUNCTION(Server, Reliable)
    void ServerPickupItem(AActor* ItemActor);

    // ���: ������ ������� ����
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayPickupFX(AActor* ItemActor);

    // Ŭ�� ����: �������� ȹ�桱 UI
    UFUNCTION(Client, Reliable)
    void ClientShowPickupUI();

protected:
    /* ====== �Է� ���ҽ� ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ArrestAction = nullptr;   // IA_Arrest (BP���� �Ҵ�)

    /* ====== ü�� Ž�� �Ķ���� ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceDistance = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceRadius = 60.f;
};

