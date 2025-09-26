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

	// ������ ��� (Ŭ���̾�Ʈ���� ȣ��)
	UFUNCTION(BlueprintCallable)
	void UseSlot1();
	UFUNCTION(BlueprintCallable)
	void UseSlot2();

	// �������� ������ ����� ó���ϴ� RPC
	UFUNCTION(Server, Reliable)
	void ServerUseItem(int32 SlotIndex);

	// ������ ��� ���� �ʱ�ȭ
	void ResetUsingItem();

	// UI ������Ʈ (Ŭ���̾�Ʈ RPC)
	UFUNCTION(Client, Reliable)
	void ClientUpdateInventoryUI();
	void ClientUpdateInventoryUI_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	// �̵�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	//���� ��ȯ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	// ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	// ������ ���
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* UseSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputAction* UseSlot2Action;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_MaxWalkSpeed)
	float CurrentMaxWalkSpeed;

	UFUNCTION()
	void OnRep_MaxWalkSpeed();

	// �浹 �̺�Ʈ
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// UI ������Ʈ �������Ʈ �̺�Ʈ
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
	void UpdateHeldItemUI(UBaseItem* NewItem);

	// ü���� ��� �ִϸ��̼�
	UFUNCTION()
	void Die();

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IsDead)
	bool bIsDead;

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION(Server, Reliable)
	void ServerResetMovementSpeed();

public:
	// �ִϸ��̼� ����
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	float Speed;

	// ������ ��� ����(����)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bUsingItem;

	// �κ��丮 (2ĭ ����)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<UBaseItem*> Inventory;

	// ���� ���õ� ���� (0 = ����1, 1 = ����2)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory")
	int32 CurrentSlotIndex = 0;

	// �̵� �ӵ�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 600.0f;

	// ī�޶�
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

	// �޸��� ȿ�� ���� �ð�
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunDuration = 5.0f;

	// Ÿ�̸� �ڵ�
	FTimerHandle RunSpeedTimerHandle;
};
