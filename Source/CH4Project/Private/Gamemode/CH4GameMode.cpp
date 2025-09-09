#include "GameMode/CH4GameMode.h"
#include "Character/CH4Character.h"
#include "PlayerController/CH4PlayerController.h"

ACH4GameMode::ACH4GameMode()
{
	DefaultPawnClass = ACH4Character::StaticClass();
	PlayerControllerClass = ACH4PlayerController::StaticClass();
}
