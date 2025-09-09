// Fill out your copyright notice in the Description page of Project Settings.


#include "Gamemode/MainGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameState/MainGameState.h"
#include "PlayerState/MainPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"
#include "NavigationSystem.h"

AMainGameMode::AMainGameMode()
{
	GameStateClass = AMainGameState::StaticClass();
	PlayerStateClass = AMainPlayerState::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 레벨에 배치된 모든 SpawnVolumeBP Actor 참조 가져오기 추후 AI, 캐릭터 스폰에 사용하기 위한 기초 로직.
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnVolume::StaticClass(), SpawnVolumes);

	// 5초 후 역할 배정
	//GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMainGameMode::AssignRoles, 5.f, false);

	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMainGameMode::TestAssignRoles8Players, 5.f, false);
	//테스트를 위한 호출
}



void AMainGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//로비 UI 등 추가 필요
}

void AMainGameMode::SetMatchState(EMatchTypes NewMatchType)
{
    AMainGameState* GS = GetGameState<AMainGameState>();
	if (GS)
	{
		GS->SetMatchState(NewMatchType);
	}
}

void AMainGameMode::AssignRoles()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS)
	{
		UE_LOG(LogTemp, Warning, TEXT("AssignRoles: GameState가 없습니다."));
		return;
	}

	int32 NumPlayers = GS->TotalPlayers;
	if (NumPlayers <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AssignRoles: 플레이어가 없습니다."));
		return;
	}

	int32 NumPolice = FMath::Max(1, NumPlayers / 4);
	int32 NumThieves = NumPlayers - NumPolice;
	int32 PoliceAssigned = 0;

	TArray<APlayerState*> PlayerStates = GameState->PlayerArray;
	Algo::RandomShuffle(PlayerStates);

	for (int32 i = 0; i < PlayerStates.Num(); ++i)
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(PlayerStates[i]);
		if (!TPS) continue;

		if (PoliceAssigned < NumPolice)
		{
			TPS->SetPlayerRole(EPlayerRole::Police);
			PoliceAssigned++;
		}
		else
		{
			TPS->SetPlayerRole(EPlayerRole::Thief);
		}
	}

	GS->RemainingPolice = NumPolice;
	GS->RemainingThieves = NumThieves;

	SetMatchState(EMatchTypes::InProgress);

	UE_LOG(LogTemp, Log, TEXT("경찰 %d명, 도둑 %d명 배정 완료."), NumPolice, NumThieves);

	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &AMainGameMode::UpdateMatchTime, 1.0f, true);
}



void AMainGameMode::UpdateMatchTime()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS || GS->MatchTypes != EMatchTypes::InProgress) return;

	GS->MatchTime = FMath::Max(0.f, GS->MatchTime - 1.f);
	if (GS->MatchTime <= 0.f)
	{
		CheckWinCondition();
	}
}

void AMainGameMode::OnThiefCaught()
{
	if (!HasAuthority()) return;

	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS) return;

	GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
	UE_LOG(LogTemp, Warning, TEXT("도둑 체포됨"));
	
	CheckWinCondition();
}

void AMainGameMode::CheckWinCondition() //승리 조건 체크 로직으로 추후 필요 시 조건 추가 필요.
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS) return;

	if (GS->RemainingThieves <= 0) //경찰 승리조건
	{
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
		
	}
	if (GS->MatchTime <= 0.f)
	{
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
		
	}
	else if (GS->RemainingPolice <= 0 )
	{
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
	}
}

void AMainGameMode::HandleGameOver()
{
	// PlayerController 처리
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		/*
		APlayerController* PC = It->Get();
		if (PC && !PC->IsLocalController())
		{
			PC->ClientTravel(TEXT("/Game/Maps/Lobby"), TRAVEL_Absolute); //추후 로비 맵 경로를 복사해와서 사용.
			//이 구간에서 로비 위젯 재출력, 위젯 파트에서 완성된 코드를 가져오거나, 출력 함수를 불러오는 것으로 재사용 가능.
		}
		*/
	}
/*
 * AI 캐릭터 및 플레이어 캐릭터 삭제용 로직
 * 만약 로비에서 캐릭터가 움직이며 대기하는 구조라면 불필요한 로직이나, 그렇게 될 시 래그돌, 혹은 사망 애니메이션으로 가사 상태로 캐릭터가 구현되어야 할 듯
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		APawn* Pawn = It->Get();
		if (Pawn)
		{
			Pawn->Destroy();
		}
	}
*/
	TArray<APlayerState*> PlayerStatesCopy = GameState->PlayerArray;
	for (int32 i = 0; i < PlayerStatesCopy.Num(); ++i)
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(PlayerStatesCopy[i]);
		if (TPS)
		{
			TPS->SetPlayerRole(EPlayerRole::Unassigned);
		}
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("게임 오버 처리 완료"));

}

void AMainGameMode::RestartGame()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (GS)
	{
		// 초기화
		GS->MatchTime = 600.f;
		GS->RemainingThieves = 0;
		GS->RemainingPolice = 0;
		SetMatchState(EMatchTypes::WaitingToStart);

		// 초기화 완료 후 값 출력
		UE_LOG(LogTemp, Warning, TEXT("=== RestartGame Debug ==="));
		UE_LOG(LogTemp, Warning, TEXT("MatchTime: %f"), GS->MatchTime);
		UE_LOG(LogTemp, Warning, TEXT("RemainingThieves: %d"), GS->RemainingThieves);
		UE_LOG(LogTemp, Warning, TEXT("RemainingPolice: %d"), GS->RemainingPolice);
		UE_LOG(LogTemp, Warning, TEXT("MatchType: %d"), static_cast<uint8>(GS->MatchTypes));

		// 플레이어별 초기화된 역할 출력
		for (APlayerState* PS : GameState->PlayerArray)
		{
			AMainPlayerState* TPS = Cast<AMainPlayerState>(PS);
			if (TPS)
			{
				UE_LOG(LogTemp, Warning, TEXT("Player %s Role: %d"), 
					*TPS->GetPlayerName(), static_cast<uint8>(TPS->PlayerRole));
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("=========================="));
	}
}



//이 로직 아래부턴 전부 기초 로직.
//추후 테스트를 거쳐야함.
void AMainGameMode::OnAICaught(APlayerController* ArrestingPlayer, APawn* AI, bool bIsCitizen)
{
    if (!AI || !ArrestingPlayer) return;

    AI->Destroy();

    AMainPlayerState* PolicePS = ArrestingPlayer->GetPlayerState<AMainPlayerState>();
    if (!PolicePS) return;

    if (bIsCitizen)
    {
        PolicePS->CurrentArrests++;
        CheckArrestLimit(PolicePS);
    }
}

void AMainGameMode::HandleArrest(APlayerController* ArrestingPlayer, APawn* TargetPawn, bool bIsThief) // 이 함수를 호출하면 대상이 도둑인지, AI 시민인지 체크함.
{
    if (!ArrestingPlayer || !TargetPawn) return;

    if (bIsThief)
    {
        OnThiefCaught();
    }
    else
    {
        bool bIsCitizen = true; // AI 시민
        OnAICaught(ArrestingPlayer, TargetPawn, bIsCitizen);
    }
}

void AMainGameMode::CheckArrestLimit(AMainPlayerState* PolicePS)
{
    if (!PolicePS) return;

    if (PolicePS->CurrentArrests >= PolicePS->MaxArrests)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: 체포 한도 초과, 사직 처리"), *PolicePS->GetPlayerName());

    	// 플레이어 폰 안전하게 제거
    	AController* Controller = PolicePS->GetOwner<AController>();
    	if (Controller)
    	{
    		APawn* Pawn = Controller->GetPawn();
    		if (Pawn)
    		{
    			Pawn->Destroy(); 
    		}
    	}

    	// 역할 초기화
    	PolicePS->SetPlayerRole(EPlayerRole::Unassigned);

    	//해당되는 UI 출력 혹은 관전 시점 전환(?)
    }
}

void AMainGameMode::UpdateMaxArrests()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS) return;

	int32 NumPolice = 0;

	// 배열 복사본 사용
	TArray<APlayerState*> PlayerStatesCopy = GameState->PlayerArray;
	for (int32 i = 0; i < PlayerStatesCopy.Num(); ++i)
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(PlayerStatesCopy[i]);
		if (TPS && TPS->PlayerRole == EPlayerRole::Police)
		{
			NumPolice++;
		}
	}
	if (NumPolice == 0) return;

	int32 TotalTargets = GS->SpawnedAI + GS->RemainingThieves;
	float ArrestMultiplier = 0.2f;
	int32 TotalArrestChances = FMath::CeilToInt(TotalTargets * ArrestMultiplier);

	TotalArrestChances = FMath::Max(TotalArrestChances, GS->RemainingThieves);
	int32 MaxArrestsPerPlayer = FMath::Max(1, TotalArrestChances / NumPolice);

	for (int32 i = 0; i < PlayerStatesCopy.Num(); ++i)
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(PlayerStatesCopy[i]);
		if (TPS && TPS->PlayerRole == EPlayerRole::Police)
		{
			TPS->MaxArrests = MaxArrestsPerPlayer;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("최대 체포 횟수 갱신: %d (경찰 %d명, 타겟 %d명, 계수 %.2f)"),
		MaxArrestsPerPlayer, NumPolice, TotalTargets, ArrestMultiplier);
}


void AMainGameMode::SpawnAI(TSubclassOf<APawn> AIPawnClass, float Radius)
{
	if (!HasAuthority() || !AIPawnClass || SpawnVolumes.Num() == 0) return;

	FVector SpawnLocation = GetRandomSpawnLocation(Radius);
	if (SpawnLocation.IsZero()) return;

	FActorSpawnParameters SpawnParams;
	APawn* NewAI = GetWorld()->SpawnActor<APawn>(AIPawnClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (NewAI)
	{
		AMainGameState* GS = GetGameState<AMainGameState>();
		if (GS)
		{
			GS->SpawnedAI++;
			UE_LOG(LogTemp, Log, TEXT("AI 스폰됨. 총 AI: %d"), GS->SpawnedAI);
		}
	}

	UpdateMaxArrests();
}

FVector AMainGameMode::GetRandomSpawnLocation(float Radius)
{
	if (SpawnVolumes.Num() == 0) return FVector::ZeroVector;

	int32 Index = FMath::RandRange(0, SpawnVolumes.Num() - 1);
	FVector Origin = SpawnVolumes[Index]->GetActorLocation();

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return FVector::ZeroVector;

	FNavLocation RandomPoint;
	if (NavSys->GetRandomPointInNavigableRadius(Origin, Radius, RandomPoint))
	{
		return RandomPoint.Location;
	}

	return FVector::ZeroVector;
}

//동작 테스트용 함수
void AMainGameMode::TestAssignRoles8Players()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (!GS) return;

	// 기존 배열 안전하게 제거
	TArray<APlayerState*> PlayerArrayCopy = GS->PlayerArray;
	for (int32 i = 0; i < PlayerArrayCopy.Num(); ++i)
	{
		if (PlayerArrayCopy[i])
		{
			PlayerArrayCopy[i]->Destroy();
		}
	}
	GS->PlayerArray.Empty();

	// 테스트용 PlayerState 생성
	for (int32 i = 0; i < 8; ++i)
	{
		AMainPlayerState* TempPS = NewObject<AMainPlayerState>(this, AMainPlayerState::StaticClass());
		if (TempPS)
		{
			TempPS->SetPlayerRole(EPlayerRole::Unassigned);
			TempPS->SetPlayerName(FString::Printf(TEXT("Player_%d"), i + 1));
			GS->PlayerArray.Add(TempPS);
		}
	}

	GS->TotalPlayers = GS->PlayerArray.Num();

	// AssignRoles 호출
	AssignRoles();

	UE_LOG(LogTemp, Warning, TEXT("=== TestAssignRoles8Players 완료 ==="));
	for (int32 i = 0; i < GS->PlayerArray.Num(); ++i)
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(GS->PlayerArray[i]);
		if (TPS)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s 역할: %d"), *TPS->GetPlayerName(), static_cast<uint8>(TPS->PlayerRole));
		}
	}
}
