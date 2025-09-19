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

protected:
    virtual void BeginPlay() override;

    // 모든 클라이언트가 동시에 게임 시작 - 테스트 싱글플레이 적용
    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartGame();
};
