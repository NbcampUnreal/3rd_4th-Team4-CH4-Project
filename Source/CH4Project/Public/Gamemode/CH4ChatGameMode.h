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

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> LobbyWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    class UUserWidget* LobbyWidget;

    UFUNCTION(BlueprintCallable, Category = "Game")
    void CheckAllPlayersReady();

    // ��� Ŭ���̾�Ʈ�� ���ÿ� ���� ���� - �׽�Ʈ �̱��÷��� ����
    UFUNCTION(BlueprintCallable, Category = "Game")
    void StartGame();
};
