#include "IngameUI/CH4UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
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
		ArrestsText->SetText(FText::FromString(FString::Printf(TEXT("KickOut : %d"), RemainingArrests)));
	}
	else
	{
		ArrestsText->SetText(FText::GetEmpty());
	}
}
void UCH4UserWidget::UpdatePlayerRole(EPlayerRole NewRole)
{
	CurrentRole = NewRole; 
	
	UpdateRemainingArrests(GetOwningPlayerState<ACH4PlayerState>()->RemainingArrests);

	if (!KillFeedBox) return;
	
	// 1) 새 TextBlock 생성
	UTextBlock* FeedText = NewObject<UTextBlock>(KillFeedBox);
	if (!FeedText) return;
	
	// 2) 표시할 내용
	FString RoleString = (NewRole == EPlayerRole::Police) ? TEXT("you are Guard") : TEXT("you are Thief");
	FeedText->SetText(FText::FromString(RoleString));

	FSlateFontInfo FontInfo;
	FontInfo.FontObject = LoadObject<UObject>(nullptr, TEXT("/Game/IngameUI/Klik-Light_Font.Klik-Light_Font"));
	FontInfo.Size = 32; // 글자 크기
	FeedText->SetFont(FontInfo);
	
	// 3) VerticalBox에 추가
	UVerticalBoxSlot* NewSlot = KillFeedBox->AddChildToVerticalBox(FeedText);
	if (NewSlot)
	{
		NewSlot->SetPadding(FMargin(2.f));
		NewSlot->SetHorizontalAlignment(HAlign_Right);
	}
	
	// 4) 5초 뒤 이 줄만 삭제
	if (UWorld* World = GetWorld())
	{
		FTimerHandle TempHandle;
		World->GetTimerManager().SetTimer(TempHandle,FTimerDelegate::CreateUObject(this, &UCH4UserWidget::RemoveKillEntry, FeedText),3.0f,false);
	}
}

void UCH4UserWidget::AddKillFeedEntry(const FString& KillerName, const FString& VictimName)
{
	// UE_LOG(LogTemp, Log, TEXT("KillFeed: %s -> %s"), *KillerName, *VictimName);
	if (!KillFeedBox) return;
	
	// 1) 새 TextBlock 생성
	UTextBlock* FeedText = NewObject<UTextBlock>(KillFeedBox);
	if (!FeedText) return;
	
	// 2) 표시할 내용
	FString Log = FString::Printf(TEXT("%s KickOut %s"), *KillerName, *VictimName);
	FeedText->SetText(FText::FromString(Log));

	FSlateFontInfo FontInfo;
	FontInfo.FontObject = LoadObject<UObject>(nullptr, TEXT("/Game/IngameUI/Klik-Light_Font.Klik-Light_Font"));
	FontInfo.Size = 32; // 글자 크기
	FeedText->SetFont(FontInfo);
	
	// 3) VerticalBox에 추가
	UVerticalBoxSlot* NewSlot = KillFeedBox->AddChildToVerticalBox(FeedText);
	if (NewSlot)
	{
		NewSlot->SetPadding(FMargin(2.f));
		NewSlot->SetHorizontalAlignment(HAlign_Right);
	}
	
	// 4) 5초 뒤 이 줄만 삭제
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
void UCH4UserWidget::UpdateInventoryUI(const TArray<UBaseItem*>& Inventory)
{
	if (!SlotImage_0 || !SlotImage_1) return;

	// 0번 슬롯
	if (Inventory.IsValidIndex(0) && Inventory[0])
	{
		SlotImage_0->SetBrushFromTexture(Inventory[0]->Icon);
		SlotImage_0->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SlotImage_0->SetBrushFromTexture(nullptr);
		SlotImage_0->SetVisibility(ESlateVisibility::Hidden);
	}

	// 1번 슬롯
	if (Inventory.IsValidIndex(1) && Inventory[1])
	{
		SlotImage_1->SetBrushFromTexture(Inventory[1]->Icon);
		SlotImage_1->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SlotImage_1->SetBrushFromTexture(nullptr);
		SlotImage_1->SetVisibility(ESlateVisibility::Hidden);
	}
}