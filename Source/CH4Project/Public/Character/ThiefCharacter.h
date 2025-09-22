#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACH4Character
{
    GENERATED_BODY()

public:
    AThiefCharacter();

    // 경찰에게 잡혔을 때 서버에서만 실행
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();
    void ServerOnCaughtByPolice_Implementation();

    // HUD/UI 클라이언트용 (경찰에게 잡혔을 때)
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();
    void ClientOnTrapped_Implementation();

protected:


};
