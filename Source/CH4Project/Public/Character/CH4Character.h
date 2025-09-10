#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "CH4Character.generated.h"

class USpringArmComponent;
class UCameraComponent;

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

	// 이동
	void MoveForward(float Value);
	void MoveRight(float Value);

	// 점프
	void JumpPressed();
	void JumpReleased();

	// 달리기
	void RunPressed();
	void RunReleased();

	// 애니메이션 관련
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsJumping;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bIsRunning;

	UPROPERTY(BlueprintReadOnly, Category = "Animation")
	float Speed;

	// 아이템 사용 여부
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Animation")
	bool bUsingItem;

	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed = 1200.0f;

	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

private:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
