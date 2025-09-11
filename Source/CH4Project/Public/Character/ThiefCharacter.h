#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"   // 부모 클래스 헤더
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACH4Character
{
    GENERATED_BODY()

public:
    AThiefCharacter();

    // 아이템 줍기
    void PickupItem(AActor* ItemActor);

    // 아이템 사용 입력
    UFUNCTION()
    void UseItemInput();

    // 서버에서 아이템 사용 처리
    UFUNCTION(Server, Reliable)
    void ServerUseItem();

protected:
    // 실제 아이템 사용 처리 (서버에서만 실행)
    void HandleUseItem(AActor* ItemActor);

    // 아이템 사용 효과를 모든 클라이언트에 알리기
    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseSpeedBoost();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseTrap();

    UFUNCTION(NetMulticast, Reliable)
    void MulticastUseClock();

    // 현재 가지고 있는 아이템
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
    AActor* HeldItem;

    // Trap 클래스 (블루프린트에서 지정)
    UPROPERTY(EditDefaultsOnly, Category = "Item")
    TSubclassOf<AActor> TrapClass;

    // 복제할 변수를 등록
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
