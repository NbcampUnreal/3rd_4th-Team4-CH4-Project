#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACharacter
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

	// 클라이언트 전체에 아이템 사용 알림
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUseItem();

	// 서버에서 체력 감소 처리
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(float DamageAmount);

protected:
	// 실제 아이템 사용 처리
	void HandleUseItem(AActor* ItemActor);

	// 현재 가지고 있는 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* HeldItem;

	// 아이템 사용 중 여부
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bUsingItem;

	// 현재 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
	float CurrentHealth;

	// 최대 체력
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	// 체력 변경 시 호출
	UFUNCTION()
	void OnRep_CurrentHealth();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
