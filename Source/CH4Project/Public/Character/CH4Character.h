#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CH4Character.generated.h"

class USpringArmComponent;
class UCameraComponent;


UCLASS()
class CH4PROJECT_API ACH4Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACH4Character();

	// 아이템 사용
	void UseItem(class AActor* ItemActor);

	UFUNCTION()
	void UseItemInput();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);

	void JumpPressed();
	void JumpReleased();
	void RunPressed();
	void RunReleased();

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	bool bUsingItem;

	// 공통 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 1200.0f;

	// 공통 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
};
