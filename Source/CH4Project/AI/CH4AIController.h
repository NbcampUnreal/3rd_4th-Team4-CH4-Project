#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "CH4AIController.generated.h"

UCLASS()
class CH4PROJECT_API ACH4AIController : public AAIController
{
	GENERATED_BODY()

public:
	ACH4AIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	private:
	FTimerHandle RandomTimer;
	void Random();
};
