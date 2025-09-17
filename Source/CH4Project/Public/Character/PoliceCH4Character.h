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

    /* ================= Item Pickup ================= */
    // 아이템 액터와 겹침 시작 → 서버 지급 시도
    UFUNCTION() void OnItemBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

    // 서버: 아이템 지급, 액터 제거
    UFUNCTION(Server, Reliable)
    void ServerPickupItem(AActor* ItemActor);

    // 모두: 아이템 사라지는 연출
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastPlayPickupFX(AActor* ItemActor);

    // 클라 전용: “아이템 획득” UI
    UFUNCTION(Client, Reliable)
    void ClientShowPickupUI();

protected:
    /* ====== 입력 리소스 ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputAction* ArrestAction = nullptr;   // IA_Arrest (BP에서 할당)

    /* ====== 체포 탐지 파라미터 ====== */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceDistance = 220.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Arrest")
    float ArrestTraceRadius = 60.f;
};

