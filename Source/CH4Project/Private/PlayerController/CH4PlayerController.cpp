#include "PlayerController/CH4PlayerController.h"
#include "EnhancedInputSubsystems.h"

ACH4PlayerController::ACH4PlayerController()
	: InputMappingContext(nullptr),
	  MoveAction(nullptr),
	  JumpAction(nullptr),
  	  LookAction(nullptr),
	  SprintAction(nullptr)
{
}

void ACH4PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem< UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				Subsystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}