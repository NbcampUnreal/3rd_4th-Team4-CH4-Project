#pragma once

#include "CoreMinimal.h"
#include "Item/BaseItem.h"
#include "CokeItem.generated.h"

UCLASS(Blueprintable)
class CH4PROJECT_API UCokeItem : public UBaseItem
{
    GENERATED_BODY()

public:
    virtual void UseItem_Implementation(ACH4Character* Character) override;
};