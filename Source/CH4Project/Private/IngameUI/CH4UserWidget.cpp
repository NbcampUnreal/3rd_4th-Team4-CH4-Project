#include "IngameUI/CH4UserWidget.h"
#include "Components/TextBlock.h"

void UCH4UserWidget::UpdateMatchTime(float NewTime)
{
	// UI 텍스트에 남은 시간을 표시
	TimeText->SetText(FText::AsNumber(FMath::RoundToInt(NewTime)));
}