#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CH4ChatGameMode.generated.h"


UCLASS()
class CH4PROJECT_API ACH4ChatGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACH4ChatGameMode();

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
    void CheckAllPlayersReady();

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
    void StartGame();

protected:
    virtual void BeginPlay() override;

    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
};
