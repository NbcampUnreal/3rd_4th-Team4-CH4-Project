#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"
#include "PoliceCH4Character.generated.h"

class UInputAction;
class UAnimMontage;
class AActor;

UCLASS()
class CH4PROJECT_API APoliceCH4Character : public ACH4Character
{
    GENERATED_BODY()

public:
    APoliceCH4Character();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;                              // [add] Tick ����
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    /* ================= Arrest (ü��) ================= */
    UFUNCTION() void OnArrestInput();

    UFUNCTION(Server, Reliable)
    void ServerTryArrest();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayArrestFX(bool bSuccess);

    UFUNCTION(Client, Reliable)
    void ClientShowArrestResultUI(bool bSuccess);

    AActor* FindArrestTarget(float TraceDistance = 220.f, float Radius = 60.f) const;

protected:
    /* ====== �Է� ���ҽ� ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ArrestAction = nullptr;

    /* ====== ü�� Ž�� �Ķ���� ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceDistance = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceRadius = 60.f;

    /* ====== ��ٿ� ====== */
    UPROPERTY(EditDefaultsOnly, Category = "Arrest|Cooldown")
    float ArrestCooldown = 1.0f;

    FTimerHandle ArrestCooldownTimerHandle_Local;
    FTimerHandle ArrestCooldownTimerHandle_Server;

    bool bArrestOnCooldown_Local = false;
    bool bArrestOnCooldown_Server = false;

    void StartArrestCooldown_Local();
    void StartArrestCooldown_Server();

    /* ====== ��Ÿ�� ��� ====== */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arrest|FX")
    UAnimMontage* ArrestMontage = nullptr;

    void PlayLocalArrestMontage();

    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayArrestMontage();

    /* ====== �̵� ��� (�߷��� ����) ====== */
    UFUNCTION()
    void OnArrestMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    void SetMovementLocked(bool bLocked);

    bool  bMovementLocked = false;
    TEnumAsByte<EMovementMode> SavedMovementMode = MOVE_Walking;
    float SavedAirControl = 0.f;
    bool  SavedOrientToMove = true;
    bool  SavedUseCtrlYaw = false;

    // XY �̼��̵� ������ ��Ŀ
    FVector2D LockAnchorXY = FVector2D::ZeroVector;
};
