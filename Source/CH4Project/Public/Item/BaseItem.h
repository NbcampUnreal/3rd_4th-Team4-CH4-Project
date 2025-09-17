#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BaseItem.generated.h"

class ACH4Character;

UCLASS(Blueprintable, Abstract)
class CH4PROJECT_API UBaseItem : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, Category = "Item")
    void UseItem(ACH4Character* Character);
    virtual void UseItem_Implementation(ACH4Character* Character);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    FText ItemName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
    UTexture2D* Icon;
};