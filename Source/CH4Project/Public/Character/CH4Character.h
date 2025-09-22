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

	// 아이템 사용과 관련된 가상 함수
	UFUNCTION(BlueprintCallable)
	virtual void UseItemInput();

	// 서버에서 아이템 사용을 처리하는 RPC
	UFUNCTION(Server, Reliable)
	virtual void ServerUseItem();
	void ServerUseItem_Implementation();

	// 실제 아이템 사용 처리
	virtual void HandleUseItem(UBaseItem* Item);

	// 아이템 획득
	UFUNCTION(BlueprintCallable, Category = "Item")
	void PickupItem(UBaseItem* Item);

	// 아이템 사용 상태 초기화
	void ResetUsingItem();

public:
	// 애니메이션 관련
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	// 아이템 사용 여부(복제)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bUsingItem;

	// 현재 가지고 있는 아이템 (복제)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_HeldItem, Category = "Item")
	UBaseItem* HeldItem;

	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 600.0f;

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

protected:
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
	class UInputAction* UseItemAction;

	// 캡슐 충돌 이벤트
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// 서버에서 아이템 줍기 요청 RPC
	UFUNCTION(Server, Reliable)
	void ServerPickupItem(AActor* ItemActor);
	void ServerPickupItem_Implementation(AActor* ItemActor);

	// HeldItem 값이 변경될 때 호출되는 함수
	UFUNCTION()
	void OnRep_HeldItem();

	// 블루프린트에서 구현하는 아이템 사용 효과 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Effect")
	void OnItemUsed(UBaseItem* Item);

	// UI 업데이트 블루프린트 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
	void UpdateHeldItemUI(UBaseItem* NewItem);

	// 속도 증가 UI 클라이언트용
	UFUNCTION(Client, Reliable)
	void ClientShowSpeedBoostUI();
	void ClientShowSpeedBoostUI_Implementation();
};
