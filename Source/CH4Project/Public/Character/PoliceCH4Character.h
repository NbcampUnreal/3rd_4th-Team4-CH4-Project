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

    /* ================= Arrest (체포) ================= */
    // 로컬 입력 → 서버 판정
    UFUNCTION() void OnArrestInput();

    // 서버: 전방 탐지 → GameMode에 위임(도둑/시민 분기)
    UFUNCTION(Server, Reliable)
    void ServerTryArrest();

    // 모두: 체포 연출(성공/실패)
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayArrestFX(bool bSuccess);

    // 클라 전용: 체포 결과 UI (체포한 본인만)
    UFUNCTION(Client, Reliable)
    void ClientShowArrestResultUI(bool bSuccess);

    // 전방 구체 트레이스로 가장 가까운 Pawn 하나
    AActor* FindArrestTarget(float TraceDistance = 220.f, float Radius = 60.f) const;

protected:
    /* ====== 입력 리소스 ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ArrestAction = nullptr;   // IA_Arrest (BP에서 할당)

    /* ====== 체포 탐지 파라미터 ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceDistance = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceRadius = 60.f;

    // === Arrest Cooldown (입력 스팸/네트워크 보호) ===
    UPROPERTY(EditDefaultsOnly, Category = "Arrest|Cooldown")
    float ArrestCooldown = 1.0f;              // 체포 쿨다운(초)

    FTimerHandle ArrestCooldownTimerHandle_Local; // 로컬(클라) 쿨다운 타이머 핸들 → 내 PC에서 연타 방지
    FTimerHandle ArrestCooldownTimerHandle_Server; // 서버 쿨다운 타이머 핸들 → 서버로 가는 RPC 스팸 방지

    // 로컬 입력 측 쿨다운(소유 클라 게이트)
    bool bArrestOnCooldown_Local = false;
    // 서버 권한 측 쿨다운(서버 게이트)
    bool bArrestOnCooldown_Server = false;

    // 로컬/서버 각각 쿨다운 시작 헬퍼
    void StartArrestCooldown_Local();
    void StartArrestCooldown_Server();

};

