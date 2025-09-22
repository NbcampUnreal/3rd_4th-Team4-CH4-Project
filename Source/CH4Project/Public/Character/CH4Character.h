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

	// ������ ���� ���õ� ���� �Լ�
	UFUNCTION(BlueprintCallable)
	virtual void UseItemInput();

	// �������� ������ ����� ó���ϴ� RPC
	UFUNCTION(Server, Reliable)
	virtual void ServerUseItem();
	void ServerUseItem_Implementation();

	// ���� ������ ��� ó��
	virtual void HandleUseItem(UBaseItem* Item);

	// ������ ȹ��
	UFUNCTION(BlueprintCallable, Category = "Item")
	void PickupItem(UBaseItem* Item);

	// ������ ��� ���� �ʱ�ȭ
	void ResetUsingItem();

public:
	// �ִϸ��̼� ����
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	// ������ ��� ����(����)
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bUsingItem;

	// ���� ������ �ִ� ������ (����)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_HeldItem, Category = "Item")
	UBaseItem* HeldItem;

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

protected:
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
	class UInputAction* UseItemAction;

	// ĸ�� �浹 �̺�Ʈ
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	// �������� ������ �ݱ� ��û RPC
	UFUNCTION(Server, Reliable)
	void ServerPickupItem(AActor* ItemActor);
	void ServerPickupItem_Implementation(AActor* ItemActor);

	// HeldItem ���� ����� �� ȣ��Ǵ� �Լ�
	UFUNCTION()
	void OnRep_HeldItem();

	// �������Ʈ���� �����ϴ� ������ ��� ȿ�� �̺�Ʈ
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|Effect")
	void OnItemUsed(UBaseItem* Item);

	// UI ������Ʈ �������Ʈ �̺�Ʈ
	UFUNCTION(BlueprintImplementableEvent, Category = "Item|UI")
	void UpdateHeldItemUI(UBaseItem* NewItem);

	// �ӵ� ���� UI Ŭ���̾�Ʈ��
	UFUNCTION(Client, Reliable)
	void ClientShowSpeedBoostUI();
	void ClientShowSpeedBoostUI_Implementation();
};
