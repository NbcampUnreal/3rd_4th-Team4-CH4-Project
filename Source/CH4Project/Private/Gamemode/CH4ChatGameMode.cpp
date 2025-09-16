#include "Gamemode/CH4ChatGameMode.h"
#include "GameState/CH4ChatGameState.h"
#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"


ACH4ChatGameMode::ACH4ChatGameMode()
{
	PlayerControllerClass = ACH4ChatPlayerController::StaticClass();
	PlayerStateClass = ACH4ChatPlayerState::StaticClass();
	GameStateClass = ACH4ChatGameState::StaticClass();
}