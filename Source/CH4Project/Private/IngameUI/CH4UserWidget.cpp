#include "IngameUI/CH4UserWidget.h"
#include "Components/TextBlock.h"

void UCH4UserWidget::UpdateMatchTime(float NewTime)
{
	TimeText->SetText(FText::FromString(FString::Printf(TEXT("%d ÃÊ"), NewTime)));
}
void UCH4UserWidget::UpdateRemainingThieves(int32 RemainingThieves)
{
	ThievesText->SetText(FText::FromString(FString::Printf(TEXT("³²Àº °æÂû : %d"), RemainingThieves)));
}
void UCH4UserWidget::UpdateRemainingPolice(int32 RemainingPolice)
{
	PoliceText->SetText(FText::FromString(FString::Printf(TEXT("³²Àº µµµÏ : %d"), RemainingPolice)));
}
void UCH4UserWidget::UpdateRemainingArrests(int32 RemainingArrests)
{
	//ArrestsText->SetText(FText::FromString(TEXT("Ã¼Æ÷È½¼ö : %d")));
	FText TestText = FText::Format(FText::FromString(TEXT("Ã¼Æ÷ È½¼ö : {0}")),FText::AsNumber(RemainingArrests));
	ArrestsText->SetText(TestText);
}