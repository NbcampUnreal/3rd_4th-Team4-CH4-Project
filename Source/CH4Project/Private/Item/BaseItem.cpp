#include "Item/BaseItem.h"
#include "Character/CH4Character.h"
#include "Engine/World.h"

void UBaseItem::UseItem_Implementation(ACH4Character* Character)
{
  
}

UWorld* UBaseItem::GetWorld() const
{
	// UObject는 Outer를 통해 World를 얻습니다.
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}