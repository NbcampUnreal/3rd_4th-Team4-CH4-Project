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
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
};
