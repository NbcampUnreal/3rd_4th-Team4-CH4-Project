#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CH4ChatGameState.generated.h"

UCLASS()
class CH4PROJECT_API ACH4ChatGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
    UFUNCTION(NetMulticast, Reliable)
    void MulticastRefreshPlayerList();

    virtual void AddPlayerState(APlayerState* PlayerState) override;
    virtual void RemovePlayerState(APlayerState* PlayerState) override;
};
