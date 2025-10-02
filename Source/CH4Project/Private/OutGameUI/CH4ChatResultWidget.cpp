#include "OutGameUI/CH4ChatResultWidget.h"
#include "Components/Button.h"
#include "PlayerController/CH4ChatPlayerController.h"

void UCH4ChatResultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (UButton* ReturnBtn = Cast<UButton>(GetWidgetFromName(TEXT("ReturnLobby"))))
    {
        ReturnBtn->OnClicked.Clear();
        ReturnBtn->OnClicked.AddDynamic(this, &UCH4ChatResultWidget::OnReturnLobbyClicked);
        UE_LOG(LogTemp, Warning, TEXT("[ResultWidget] ReturnLobby button BOUND"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[ResultWidget] Button 'ReturnLobby' NOT FOUND"));
    }
}

void UCH4ChatResultWidget::OnReturnLobbyClicked()
{
    if (ACH4ChatPlayerController* PC = Cast<ACH4ChatPlayerController>(GetOwningPlayer()))
    {
        PC->OnResultReturnToLobbyClicked();
    }
}