#include "OutGameUI/CH4ChatUserWidget.h"
#include "Components/Button.h"
#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"

void UCH4ChatUserWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ReadyMatch)
        ReadyMatch->OnClicked.AddDynamic(this, &UCH4ChatUserWidget::HandleReadyClicked);
    if (GameExit)
        GameExit->OnClicked.AddDynamic(this, &UCH4ChatUserWidget::HandleExitClicked);
}

void UCH4ChatUserWidget::HandleReadyClicked()
{
    if (ACH4ChatPlayerController* PC = GetOwningPlayer<ACH4ChatPlayerController>())
    {
        bool bNewReady = true;
        if (ACH4ChatPlayerState* PS = PC->GetPlayerState<ACH4ChatPlayerState>())
        {
            bNewReady = !PS->IsReady();
        }
        PC->SetReady(bNewReady);
    }
}

void UCH4ChatUserWidget::HandleExitClicked()
{
    if (ACH4ChatPlayerController* PC = GetOwningPlayer<ACH4ChatPlayerController>())
    {
        PC->ExitGame();
    }
}