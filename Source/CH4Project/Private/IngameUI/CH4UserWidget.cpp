#include "IngameUI/CH4UserWidget.h"
#include "Components/TextBlock.h"
#include "PlayerState/CH4PlayerState.h"

void UCH4UserWidget::UpdateMatchTime(float MatchTime)
{
	TimeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f s"), MatchTime)));
}
void UCH4UserWidget::UpdateRemainingThieves(int32 RemainingThieves)
{
	ThievesText->SetText(FText::FromString(FString::Printf(TEXT("Thief : %d"), RemainingThieves)));
}
void UCH4UserWidget::UpdateRemainingPolice(int32 RemainingPolice)
{
	PoliceText->SetText(FText::FromString(FString::Printf(TEXT("Guard : %d"), RemainingPolice)));
}
void UCH4UserWidget::UpdateRemainingArrests(int32 RemainingArrests)
{
	ArrestsText->SetText(FText::FromString(FString::Printf(TEXT("Arrest : %d"), RemainingArrests)));
	// FText TestText = FText::Format(FText::FromString(TEXT("Arrest : {0}")),FText::AsNumber(RemainingArrests));
	// ArrestsText->SetText(TestText);
}
void UCH4UserWidget::UpdatePlayerRole(EPlayerRole NewRole)
{
	FString RoleString = (NewRole == EPlayerRole::Police) ? TEXT("you are Guard") : TEXT("you are Thief");
	StatusText->SetText(FText::FromString(RoleString));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ClearTextTimerHandle); // 기존 타이머 제거
		World->GetTimerManager().SetTimer(ClearTextTimerHandle, this, &UCH4UserWidget::ClearStatusText, 3.0f, false);
	}
}
void UCH4UserWidget::ClearStatusText()
{
	if (StatusText)
	{
		StatusText->SetText(FText::GetEmpty());
	}
}
