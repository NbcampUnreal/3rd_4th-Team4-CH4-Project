#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "CH4Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UBaseItem;
class APickUp;
struct FInputActionValue;


UCLASS()
class CH4PROJECT_API ACH4Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACH4Character();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// 아이템 사용 (클라이언트에서 호출)
	UFUNCTION(BlueprintCallable)
	void UseSlot1();
	UFUNCTION(BlueprintCallable)
	void UseSlot2();

	// 서버에서 아이템 사용을 처리하는 RPC
	UFUNCTION(Server, Reliable)
	void ServerUseItem(int32 SlotIndex);

	// 아이템 사용 상태 초기화
	void ResetUsingItem();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// 이동
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	//시점 전환
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// 점프
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	// 아이템 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* UseSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* UseSlot2Action;

	// 충돌 이벤트
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// UI 업데이트 블루프린트 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
	void UpdateHeldItemUI(UBaseItem* NewItem);

	// 체포시 사망 애니메이션
	UFUNCTION()
	void Die();

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IsDead)
	bool bIsDead;

	UFUNCTION()
	void OnRep_IsDead();

	// 애니메이션 인스턴스에서 사용할 기절 상태
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsStunned = false;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	class UAnimMontage* StunMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* DieMontage;

	// 애니메이션이 재생되도록 지시
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayStunMontage();

	UFUNCTION()
	void RemoveCharacterAfterDeath();

	// 애니메이션이 끝났을 때 호출
	void ResetStunState();

	FTimerHandle DestroyTimerHandle;



public:
	// 애니메이션 관련
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	float Speed;

	// 아이템 사용 여부(복제)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bUsingItem;

	// 인벤토리 (2칸 고정)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UBaseItem*> Inventory;

	// 현재 선택된 슬롯 (0 = 슬롯1, 1 = 슬롯2)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	int32 CurrentSlotIndex = 0;

	// 서버에서만 호출, MaxWalkSpeed를 변경 및 클라이언트에게 복제
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCharacterMaxWalkSpeed(float NewMaxWalkSpeed);

	// UI 업데이트 (클라이언트 RPC)
	UFUNCTION(Client, Reliable)
	void ClientUpdateInventoryUI();
	void ClientUpdateInventoryUI_Implementation();

	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 600.0f;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_MaxWalkSpeed)
	float CurrentMaxWalkSpeed;

	UFUNCTION()
	void OnRep_MaxWalkSpeed();

	UFUNCTION(Server, Reliable)
	void ServerResetMovementSpeed();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerPlayStunAnimation();

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	// 타이머 핸들
	FTimerHandle RunSpeedTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerHandleDeath();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayDeathAnimation();

	UFUNCTION(Client, Reliable)
	void ClientAddItem(FName ItemName);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateItem(FName ItemName);
};
