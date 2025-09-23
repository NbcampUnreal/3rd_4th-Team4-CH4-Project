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

protected:
    /* ====== �Է� ���ҽ� ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ArrestAction = nullptr;   // IA_Arrest (BP���� �Ҵ�)

    /* ====== ü�� Ž�� �Ķ���� ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceDistance = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceRadius = 60.f;

    // === Arrest Cooldown (�Է� ����/��Ʈ��ũ ��ȣ) ===
    UPROPERTY(EditDefaultsOnly, Category = "Arrest|Cooldown")
    float ArrestCooldown = 1.0f;              // ü�� ��ٿ�(��)

    FTimerHandle ArrestCooldownTimerHandle_Local; // ����(Ŭ��) ��ٿ� Ÿ�̸� �ڵ� �� �� PC���� ��Ÿ ����
    FTimerHandle ArrestCooldownTimerHandle_Server; // ���� ��ٿ� Ÿ�̸� �ڵ� �� ������ ���� RPC ���� ����

    // ���� �Է� �� ��ٿ�(���� Ŭ�� ����Ʈ)
    bool bArrestOnCooldown_Local = false;
    // ���� ���� �� ��ٿ�(���� ����Ʈ)
    bool bArrestOnCooldown_Server = false;

    // ����/���� ���� ��ٿ� ���� ����
    void StartArrestCooldown_Local();
    void StartArrestCooldown_Server();

};

