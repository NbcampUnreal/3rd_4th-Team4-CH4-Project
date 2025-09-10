#include "CH4AIController.h"
#include "NavigationSystem.h"

ACH4AIController::ACH4AIController()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	SetNetUpdateFrequency(5.0f);       // 초당 5번
	SetMinNetUpdateFrequency(1.0f);    // 최소 1Hz
	
}

void ACH4AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (GetNetMode() == NM_Client) return;

	// 2~5초마다 랜덤 이동 실행
	GetWorld()->GetTimerManager().SetTimer(
		RandomTimer, this, &ACH4AIController::Random,FMath::FRandRange(2.0f, 5.0f), true
	);
}

void ACH4AIController::Random()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return;

	FNavLocation RandomPoint;
	if (NavSys->GetRandomPointInNavigableRadius(MyPawn->GetActorLocation(), 500.0f, RandomPoint))
	{
		MoveToLocation(RandomPoint.Location);
	}
}