#pragma once

#include "CoreMinimal.h"
#include "Item/BaseItem.h"
#include "ClockItem.generated.h"

UCLASS(Blueprintable)
class CH4PROJECT_API UClockItem : public UBaseItem
{
    GENERATED_BODY()

public:
    virtual void UseItem_Implementation(ACH4Character* Character) override;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Item")
    float TimeToReduce;
};