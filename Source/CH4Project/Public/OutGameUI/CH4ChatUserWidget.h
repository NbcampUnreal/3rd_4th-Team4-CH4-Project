#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH4ChatUserWidget.generated.h"

UCLASS()
class CH4PROJECT_API UCH4ChatUserWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    class UButton* ReadyMatch;

    UPROPERTY(meta = (BindWidget))
    class UButton* GameExit;

    UFUNCTION()
    void HandleReadyClicked();

    UFUNCTION()
    void HandleExitClicked();
};