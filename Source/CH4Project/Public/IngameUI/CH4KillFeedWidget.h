// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CH4KillFeedWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class CH4PROJECT_API UCH4KillFeedWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FeedText;
private:
	FTimerHandle RemoveTimerHandle;
};
