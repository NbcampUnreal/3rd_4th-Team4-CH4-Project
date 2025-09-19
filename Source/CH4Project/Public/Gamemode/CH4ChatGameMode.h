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

    // ��� Ŭ���̾�Ʈ�� ���ÿ� ���� ���� - �׽�Ʈ �̱��÷��� ����
    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartGame();
};
