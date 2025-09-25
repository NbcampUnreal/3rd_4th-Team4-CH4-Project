#include "IngameUI/CH4UserWidget.h"
#include "IngameUI/CH4KillFeedWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
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
	if (CurrentRole == EPlayerRole::Police)
	{
		ArrestsText->SetText(FText::FromString(FString::Printf(TEXT("Arrest : %d"), RemainingArrests)));
	}
	else
	{
		ArrestsText->SetText(FText::GetEmpty());
	}
}
void UCH4UserWidget::UpdatePlayerRole(EPlayerRole NewRole)
{
	CurrentRole = NewRole; 
	
	FString RoleString = (NewRole == EPlayerRole::Police) ? TEXT("you are Guard") : TEXT("you are Thief");
	StatusText->SetText(FText::FromString(RoleString));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ClearTextTimerHandle);
		World->GetTimerManager().SetTimer(ClearTextTimerHandle, this, &UCH4UserWidget::ClearStatusText, 3.0f, false);
	}
	
	UpdateRemainingArrests(GetOwningPlayerState<ACH4PlayerState>()->RemainingArrests);
}
void UCH4UserWidget::ClearStatusText()
{
	if (StatusText)
	{
		StatusText->SetText(FText::GetEmpty());
	}
}
void UCH4UserWidget::AddKillFeedEntry(const FString& KillerName, const FString& VictimName)
{
	// UE_LOG(LogTemp, Log, TEXT("KillFeed: %s -> %s"), *KillerName, *VictimName);
	if (!KillFeedBox) return;
	
	// 1) �� TextBlock ����
	UTextBlock* FeedText = NewObject<UTextBlock>(KillFeedBox);
	if (!FeedText) return;
	
	// 2) ǥ���� ����
	FString Log = FString::Printf(TEXT("%s KickOut %s"), *KillerName, *VictimName);
	FeedText->SetText(FText::FromString(Log));
	
	// 3) VerticalBox�� �߰�
	UVerticalBoxSlot* NewSlot = KillFeedBox->AddChildToVerticalBox(FeedText);
	if (NewSlot)
	{
		NewSlot->SetPadding(FMargin(2.f));
		NewSlot->SetHorizontalAlignment(HAlign_Left);
	}
	
	// 4) 5�� �� �� �ٸ� ����
	if (UWorld* World = GetWorld())
	{
		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(TempHandle,FTimerDelegate::CreateUObject(this, &UCH4UserWidget::RemoveKillEntry, FeedText),5.0f,false);
	}

}
void UCH4UserWidget::RemoveKillEntry(UTextBlock* Entry)
{
	if (KillFeedBox && Entry)
	{
		KillFeedBox->RemoveChild(Entry);
	}
}
