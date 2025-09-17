#include "IngameUI/CH4UserWidget.h"
#include "Components/TextBlock.h"

void UCH4UserWidget::UpdateMatchTime(float NewTime)
{
	TimeText->SetText(FText::AsNumber(NewTime));
}
void UCH4UserWidget::UpdateRemainingThieves(int32 RemainingThieves)
{
	ThievesText->SetText(FText::AsNumber(RemainingThieves));
}
void UCH4UserWidget::UpdateRemainingPolice(int32 RemainingPolice)
{
	PoliceText->SetText(FText::AsNumber(RemainingPolice));
}