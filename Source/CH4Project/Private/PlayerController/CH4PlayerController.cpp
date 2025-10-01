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
		UE_LOG(LogTemp, Warning, TEXT("[Client_UpdateMatchData] FinalWinner 업데이트 완료: %d (클라: %s)"),
		static_cast<uint8>(GI->FinalWinner), *GetName());
		
		GI->LastRoles.Empty();
		
		for (const FPlayerRoleData& Data : Roles)
		{
			GI->LastRoles.Add(Data.PlayerName, Data.Role);
			UE_LOG(LogTemp, Warning, TEXT("[Client_UpdateMatchData] 역할 업데이트: %s -> %d (클라: %s)"),
				*Data.PlayerName,
				static_cast<uint8>(Data.Role),
				*GetName());
		}
		
		GI->LastMatchState = EMatchTypes::GameOver;
		UE_LOG(LogTemp, Warning, TEXT("[Client_UpdateMatchData] LastMatchState 업데이트 완료: %d (클라: %s)"),
			static_cast<uint8>(GI->LastMatchState),
			*GetName());
	}
}
