#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH4ChatResultWidget.generated.h"

UCLASS()
class CH4PROJECT_API UCH4ChatResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    class UButton* ReturnLobby;

    UFUNCTION()
    void OnReturnLobbyClicked();

};
