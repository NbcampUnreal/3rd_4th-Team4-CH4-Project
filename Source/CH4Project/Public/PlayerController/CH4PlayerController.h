#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CH4PlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UCH4UserWidget;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* UseSlot1Action;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	UInputAction* UseSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UCH4UserWidget> HUDWidgetClass;
	UPROPERTY()
	UCH4UserWidget* MyHUDWidget;

	UFUNCTION(Client, Reliable)
	void Client_UpdateMatchData(EWinTeam Winner, const TArray<FPlayerRoleData>& Roles);
	
protected:
	virtual void BeginPlay() override;
};
