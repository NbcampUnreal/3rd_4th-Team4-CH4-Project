#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH4UserWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UImage;
class UBaseItem;
/**
 * 
 */
UCLASS()
class CH4PROJECT_API UCH4UserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateMatchTime(float MatchTime);
	UFUNCTION(BlueprintCallable)
	void UpdateRemainingThieves(int32 RemainingThieves);
	UFUNCTION(BlueprintCallable)
	void UpdateRemainingPolice(int32 RemainingPolice);
	UFUNCTION(BlueprintCallable)
	void UpdateRemainingArrests(int32 RemainingArrests);
	UFUNCTION(BlueprintCallable)
	void UpdatePlayerRole(EPlayerRole NewRole);
	UFUNCTION(BlueprintCallable)
	void AddKillFeedEntry(const FString& KillerName, const FString& VictimName);
	UFUNCTION(BlueprintCallable)
	void UpdateInventoryUI(const TArray<UBaseItem*>& Inventory);

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ThievesText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PoliceText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ArrestsText;
	UPROPERTY(meta = (BindWidget))
	UVerticalBox* KillFeedBox;
	UPROPERTY(meta = (BindWidget))
	UImage* SlotImage_0;
	UPROPERTY(meta = (BindWidget))
	UImage* SlotImage_1;

private:
	EPlayerRole CurrentRole; 
	
	void RemoveKillEntry(UTextBlock* Entry);

};
