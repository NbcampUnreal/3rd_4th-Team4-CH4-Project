#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "CH4UserWidget.generated.h"

/**
 * 
 */
UCLASS()
class CH4PROJECT_API UCH4UserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void UpdateMatchTime(float NewTime);
	UFUNCTION(BlueprintCallable)
	void UpdateRemainingThieves(int32 RemainingThieves);
	UFUNCTION(BlueprintCallable)
	void UpdateRemainingPolice(int32 RemainingPolice);
	UFUNCTION(BlueprintCallable)
	void UpdateRemainingArrests(int32 RemainingArrests);
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ThievesText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PoliceText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ArrestsText;
};
