#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"   // 부모 클래스 헤더
#include "Item/BaseItem.h"
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACH4Character
{
    GENERATED_BODY()

public:
    AThiefCharacter();

    // 아이템 사용 입력
    virtual void UseItemInput() override;
    void ServerUseItem_Implementation();

    // 아이템 줍기
    UFUNCTION(BlueprintCallable)
    void PickupItem(UBaseItem* Item);

    // 경찰에게 잡혔을 때 (서버에서만 실행)
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();

    // 아이템 효과를 모든 클라이언트에 복제
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseClock();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseTrap();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseSpeedBoost();

    // HUD 및 UI를 위한 클라이언트 전용 함수
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();

    UFUNCTION(Client, Reliable)
    void ClientShowSpeedBoostUI();


protected:
    // 실제 아이템 사용 처리 (서버에서만 실행)
    void HandleUseItem(UBaseItem* Item);

    void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // 현재 가지고 있는 아이템
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    UBaseItem* HeldItem;

    // 복제할 변수를 등록
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 블루프린트에서 구현하는 이벤트들
    UFUNCTION(BlueprintImplementableEvent)
    void OnClockEffect();

    UFUNCTION(BlueprintImplementableEvent)
    void OnTrapEffect();

    UFUNCTION(BlueprintImplementableEvent)
    void OnSpeedBoostEffect();

    UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
    void ShowSpeedBoostUI(); // 소유자 HUD에만 보여줄 UI
    };
