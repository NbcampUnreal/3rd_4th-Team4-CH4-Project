#include "IngameUI/CH4UserWidget.h"
#include "Components/TextBlock.h"

void UCH4UserWidget::UpdateMatchTime(float NewTime)
{
	// UI �ؽ�Ʈ�� ���� �ð��� ǥ��
	TimeText->SetText(FText::AsNumber(FMath::RoundToInt(NewTime)));
}