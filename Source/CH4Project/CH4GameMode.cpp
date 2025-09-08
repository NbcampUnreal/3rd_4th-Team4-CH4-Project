#include "CH4GameMode.h"
#include "CH4Character.h"
#include "CH4PlayerController.h"

ACH4GameMode::ACH4GameMode()
{
	DefaultPawnClass = ACH4Character::StaticClass();
	PlayerControllerClass = ACH4PlayerController::StaticClass();
}
