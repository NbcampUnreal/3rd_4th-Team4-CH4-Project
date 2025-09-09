#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class CH4PROJECT_API ACH4PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ACH4PlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputMappingContext* InputMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* SprintAction;

protected:
	virtual void BeginPlay() override;
};
