#include "PlayerController/CH4PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "IngameUI/CH4UserWidget.h"
#include "GameInstance/CH4GameInstance.h"

ACH4PlayerController::ACH4PlayerController()
	: InputMappingContext(nullptr),
	  MoveAction(nullptr),
	  JumpAction(nullptr),
  	  LookAction(nullptr),
	  SprintAction(nullptr),
	UseSlot1Action(nullptr),
	UseSlot2Action(nullptr),
	MyHUDWidget(nullptr)
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

	if (IsLocalController() && HUDWidgetClass)
	{
		UCH4UserWidget* HUDWidget = CreateWidget<UCH4UserWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			MyHUDWidget = HUDWidget;
		}
	}
}

void ACH4PlayerController::Client_UpdateMatchData_Implementation(EWinTeam Winner, const TArray<FPlayerRoleData>& Roles)
{
	if (UCH4GameInstance* GI = GetGameInstance<UCH4GameInstance>())
	{
		GI->FinalWinner = Winner;
		GI->LastRoles.Empty();

		for (const FPlayerRoleData& Data : Roles)
		{
			GI->LastRoles.Add(Data.PlayerName, Data.Role);
		}

		GI->LastMatchState = EMatchTypes::GameOver;
	}
}