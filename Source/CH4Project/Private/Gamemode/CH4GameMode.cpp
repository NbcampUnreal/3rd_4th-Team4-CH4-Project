// Fill out your copyright notice in the Description page of Project Settings.


#include "Gamemode/CH4GameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameState/CH4GameStateBase.h"
#include "PlayerState/CH4PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"
#include "NavigationSystem.h"
#include "SpawnVolume/BaseSpawnVolume.h"
#include "SpawnVolume/PlayerSpawnVolume.h"
#include "SpawnVolume/AISpawnVolume.h"

ACH4GameMode::ACH4GameMode()
{
	GameStateClass = ACH4GameStateBase::StaticClass();
	PlayerStateClass = ACH4PlayerState::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}

void ACH4GameMode::BeginPlay()
{
	Super::BeginPlay();


	// 레벨에 배치된 모든 SpawnVolume Actor 참조 가져오기 추후 AI, 캐릭터 스폰에 사용하기 위한 기초 로직.
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseSpawnVolume::StaticClass(), SpawnVolumes);


	
	// 5초 후 역할 배정 
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ACH4GameMode::AssignRoles, 5.f, false);
	//만일 이 구조대로 사용한다면 AI의 스폰, 역할 부여를 위해 AssingRoles를 우선 호출하고, 추후에 타이머가 돌아가는 구조가 필요함.

	
	//GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ACH4GameMode::TestAssignRoles8Players, 5.f, false);
	//테스트를 위한 호출



}



void ACH4GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//로비에 들어온 후 레벨로 넘어오는 구조인 상황에서, 로그인 구조를 게임모드에서 관리(?)
}

void ACH4GameMode::SetMatchState(EMatchTypes NewMatchType)
{
    ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (GS)
	{
		GS->SetMatchState(NewMatchType);
	}
}

void ACH4GameMode::AssignRoles()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
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

    TArray<APlayerState*> PlayerStates = GS->PlayerArray;
	Algo::RandomShuffle(PlayerStates);

	for (int32 i = 0; i < PlayerStates.Num(); ++i)
	{
		ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PlayerStates[i]);
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

	
	//SpawnActors(AIClassesToSpawn, AISpawnRadius);
	//내일 작업 시 멤버 변수 수정 필요.
	
	SetMatchState(EMatchTypes::InProgress);

	UE_LOG(LogTemp, Log, TEXT("경찰 %d명, 도둑 %d명 배정 완료."), NumPolice, NumThieves);
	
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &ACH4GameMode::UpdateMatchTime, 1.0f, true);
}



void ACH4GameMode::UpdateMatchTime()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS || GS->MatchTypes != EMatchTypes::InProgress) return;

	GS->MatchTime = FMath::Max(0.f, GS->MatchTime - 1.f);
	if (GS->MatchTime <= 0.f)
	{
		CheckWinCondition();
	}
}

void ACH4GameMode::OnThiefCaught(APawn* ThiefPawn) // 추후 정상적으로 해당 캐릭터가 해제되는지 확인 필요.
{
	if (!HasAuthority()) return;

	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;

	GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
	UE_LOG(LogTemp, Warning, TEXT("도둑 체포됨"));
	
	if (ThiefPawn)
	{
		AController* Controller = ThiefPawn->GetController();
		if (Controller)
		{
			Controller->UnPossess();
		}

		ThiefPawn->Destroy();
	}
	
	CheckWinCondition();
}

void ACH4GameMode::CheckWinCondition() //승리 조건 체크 로직으로 추후 필요 시 조건 추가 필요.
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
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

void ACH4GameMode::HandleGameOver()
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
		ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PlayerStatesCopy[i]);
		if (TPS)
		{
			TPS->SetPlayerRole(EPlayerRole::Unassigned);
		}
	}

	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	UE_LOG(LogTemp, Warning, TEXT("게임 오버 처리 완료"));

}

void ACH4GameMode::RestartGame()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
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
			ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PS);
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
void ACH4GameMode::OnAICaught(APlayerController* ArrestingPlayer, APawn* AI, bool bIsCitizen)
{
    if (!AI || !ArrestingPlayer) return;

    AI->Destroy();

    ACH4PlayerState* PolicePS = ArrestingPlayer->GetPlayerState<ACH4PlayerState>();
    if (!PolicePS) return;

    if (bIsCitizen)
    {
        PolicePS->CurrentArrests++;
        CheckArrestLimit(PolicePS);
    }
}


void ACH4GameMode::HandleArrest(APlayerController* ArrestingPlayer, APawn* TargetPawn)
{
	if (!ArrestingPlayer || !TargetPawn) return;

	ACH4PlayerState* MPS = TargetPawn->GetPlayerState<ACH4PlayerState>();
	if (!MPS) //if (MPS && MPS->PlayerRole == EPlayerRole::Citizen) AI에 플레이어롤 시티즌을 추가하면 이것으로 수정 필요
	{
		OnAICaught(ArrestingPlayer, TargetPawn, true);
		return;
	}

	switch (MPS->PlayerRole)
	{
	case EPlayerRole::Thief:
		OnThiefCaught(TargetPawn);
		break;

	case EPlayerRole::Police:
		UE_LOG(LogTemp, Warning, TEXT("경찰은 체포 대상이 아님!"));
		break;

	default:
		UE_LOG(LogTemp, Warning, TEXT("경고 해결을 위한 버전입니다.: %d"), (int32)MPS->PlayerRole);
		break;
	}
}


void ACH4GameMode::CheckArrestLimit(ACH4PlayerState* PolicePS)
{
    if (!PolicePS) return;

    if (PolicePS->CurrentArrests >= PolicePS->MaxArrests)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: 체포 한도 초과, 사직 처리"), *PolicePS->GetPlayerName());

    	// 경찰 플레이어 폰 안전하게 제거
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

void ACH4GameMode::UpdateMaxArrests()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;

	int32 NumPolice = 0;

	// 배열 복사본 사용
	TArray<APlayerState*> PlayerStatesCopy = GameState->PlayerArray;
	for (int32 i = 0; i < PlayerStatesCopy.Num(); ++i)
	{
		ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PlayerStatesCopy[i]);
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
		ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PlayerStatesCopy[i]);
		if (TPS && TPS->PlayerRole == EPlayerRole::Police)
		{
			TPS->MaxArrests = MaxArrestsPerPlayer;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("최대 체포 횟수 갱신: %d (경찰 %d명, 타겟 %d명, 계수 %.2f)"),
		MaxArrestsPerPlayer, NumPolice, TotalTargets, ArrestMultiplier);
}

/*
void ACH4GameMode::SpawnAI(TSubclassOf<APawn> AIPawnClass, float Radius)
{
	if (!HasAuthority() || !AIPawnClass || SpawnVolumes.Num() == 0) return;

	FVector SpawnLocation = GetRandomSpawnLocation(Radius);
	if (SpawnLocation.IsZero()) return;

	FActorSpawnParameters SpawnParams;
	APawn* NewAI = GetWorld()->SpawnActor<APawn>(AIPawnClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (NewAI)
	{
		ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
		if (GS)
		{
			GS->SpawnedAI++;
			UE_LOG(LogTemp, Log, TEXT("AI 스폰됨. 총 AI: %d"), GS->SpawnedAI);
		}
	}

	UpdateMaxArrests();
}

FVector ACH4GameMode::GetRandomSpawnLocation(float Radius)
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
*/

//스폰 구조 통합 관리하는 로직
void ACH4GameMode::SpawnActors(TArray<TSubclassOf<APawn>> AIClasses, float AISpawnRadius)
{
    if (!HasAuthority() || SpawnVolumes.Num() == 0) 
    {
    	UE_LOG(LogTemp, Warning, TEXT("스폰 볼륨이 없습니다."));
    	return;
    }
    TArray<ABaseSpawnVolume*> AISpawnVolumes;
    TArray<ABaseSpawnVolume*> PlayerSpawnVolumes;

    // AI와 플레이어의 스폰볼륨 타입 구분
	for (AActor* Actor : SpawnVolumes)
	{
		if (AAISpawnVolume* AIVol = Cast<AAISpawnVolume>(Actor))
		{
			AISpawnVolumes.Add(AIVol);
		}
		else if (APlayerSpawnVolume* PVol = Cast<APlayerSpawnVolume>(Actor))
		{
			PlayerSpawnVolumes.Add(PVol);
		}
	}

	//AI의 클래스 순회
    for (TSubclassOf<APawn> AIClass : AIClasses)
    {
        if (AISpawnVolumes.Num() == 0) break; //AI 스폰 볼륨이 없을 경우 종료

        int32 Index = FMath::RandRange(0, AISpawnVolumes.Num() - 1);
        FVector Origin = AISpawnVolumes[Index]->GetActorLocation(); // AI 볼륨의 인덱스 중 랜덤 위치를 선택, 선택한 볼륨의 좌표 확보

    	//네비게이션 시스템 접근을 통해 네비 메쉬가 없을 경우 스폰 불가
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (!NavSys) continue;

        FNavLocation RandomPoint;
        if (NavSys->GetRandomPointInNavigableRadius(Origin, AISpawnRadius, RandomPoint))
        {
            FActorSpawnParameters SpawnParams;
            APawn* NewAI = GetWorld()->SpawnActor<APawn>(AIClass, RandomPoint.Location, FRotator::ZeroRotator, SpawnParams);
            if (NewAI)
            {
                ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
                if (GS)
                {
                    GS->SpawnedAI++;
                    UE_LOG(LogTemp, Log, TEXT("AI 스폰됨. 총 AI: %d"), GS->SpawnedAI);
                }
            }
        }
    }
	
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;
	TArray<APlayerState*> Players = GS->PlayerArray; //게임 스테이트에서 자동으로 수집하는 플레이어어래이 배열을 가져와 Players로 지정.

    Algo::RandomShuffle(PlayerSpawnVolumes);
    Algo::RandomShuffle(Players); //플레이어 스폰볼륨과 플레이어들의 배열을 섞는다.

    int32 SpawnCount = FMath::Min(PlayerSpawnVolumes.Num(), Players.Num()); // 스폰 볼륨과, 플레이어 배열 수 중 작은 숫자만큼 스폰카운트로 재정의함.
	//핵심은 언제나 레벨 내에 배치된 스폰 볼륨이 플레이어 수보다 많아야함. (Player < PlayerSpawnVolume 액터)
	//수동 배치로 플레이어 스폰 액터 BP를 적절한 위치에 많이 배치해 주세요.

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        FVector SpawnLoc = PlayerSpawnVolumes[i]->GetActorLocation(); //플레이어 스폰 볼륨의 위치를 가져온 후 기본 회전은 정면을 바라보게 설정
        FRotator SpawnRot = FRotator::ZeroRotator;

        // 플레이어 폰 스폰
        AController* PlayerController = Cast<AController>(Players[i]->GetOwner()); //플레이어 컨트롤러를 확보
        if (PlayerController)
        {
            APawn* PlayerPawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, SpawnLoc, SpawnRot); //실제  Pawn 스폰 과정
            if (PlayerPawn)
            {
                PlayerController->Possess(PlayerPawn); //컨트롤러가 스폰된 Pawn을 소유하고 빙의함.
                UE_LOG(LogTemp, Log, TEXT("%s 스폰 완료"), *Players[i]->GetPlayerName());
            }
        }
    } // for문으론 끝까지 한 번 순회한 후 종료되기에, 이론상 기존에 사용했던 스폰 볼륨 위치에는 플레이어가 스폰되지 않는다.
	//따라서 실제 테스트 과정이 필요.
	UpdateMaxArrests();

}

//동작 테스트용 함수
void ACH4GameMode::TestAssignRoles8Players()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
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
		ACH4PlayerState* TempPS = NewObject<ACH4PlayerState>(this, ACH4PlayerState::StaticClass());
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
		ACH4PlayerState* TPS = Cast<ACH4PlayerState>(GS->PlayerArray[i]);
		if (TPS)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s 역할: %d"), *TPS->GetPlayerName(), static_cast<uint8>(TPS->PlayerRole));
		}
	}
}
