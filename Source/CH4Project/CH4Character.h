#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CH4Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class FInputActionValue;

UCLASS()
class CH4PROJECT_API ACH4Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACH4Character();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& value);
	void Jump(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void Sprint(const FInputActionValue& value);

};
