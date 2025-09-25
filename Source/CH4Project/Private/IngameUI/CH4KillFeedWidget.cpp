#include "IngameUI/CH4KillFeedWidget.h"
#include "Components/TextBlock.h"

void UCH4KillFeedWidget::NativeConstruct()
{

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(RemoveTimerHandle, this, &UCH4KillFeedWidget::RemoveFromParent, 5.0f, false);
	}
}