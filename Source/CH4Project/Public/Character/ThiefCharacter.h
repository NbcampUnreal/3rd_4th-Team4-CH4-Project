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
    // 생성자
    AThiefCharacter();

    //아이템 사용 입력, 로컬에서 호출
    virtual void UseItemInput() override;

    // 아이템 줍기
    UFUNCTION(BlueprintCallable, Category = "Item")
    void PickupItem(UBaseItem* Item);

    // 서버에서 아이템 사용 요청 RPC
    virtual void ServerUseItem_Implementation() override;

    // 서버에서 아이템 줍기 요청 RPC
    UFUNCTION(Server, Reliable)
    void ServerPickupItem(AActor* ItemActor);
    void ServerPickupItem_Implementation(AActor* ItemActor);

    // 경찰에게 잡혔을 때 서버에서만 실행
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();
    void ServerOnCaughtByPolice_Implementation();

    // HUD/UI 클라이언트용
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();
    void ClientOnTrapped_Implementation();

    // 속도 증가 UI 클라이언트용
    UFUNCTION(Client, Reliable)
    void ClientShowSpeedBoostUI();
    void ClientShowSpeedBoostUI_Implementation();

protected:
    // 캡슐 충돌 이벤트
    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    //실제 아이템 사용 처리 (서버에서만 실행)
    virtual void HandleUseItem(UBaseItem* Item);

    //사용 중인 아이템 초기화 함수
    void ResetUsingItem();

    // 현재 가지고 있는 아이템
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_HeldItem, Category = "Item")
    UBaseItem* HeldItem;

    // HeldItem 값이 변경될 때 자동으로 호출되는 함수
    UFUNCTION()
    void OnRep_HeldItem();

    // 복제 등록
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 블루프린트에서 구현하는 아이템 사용 효과 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Item|Effect")
    void OnItemUsed(UBaseItem* Item);

    // 소유자 UI: 속도 증가 표시 (블루프린트에서 구현)
    UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
    void ShowSpeedBoostUI();

    // UI 업데이트 블루프린트 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
    void UpdateHeldItemUI(UBaseItem* NewItem);
};