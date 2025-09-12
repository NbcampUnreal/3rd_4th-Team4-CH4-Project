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
#include "GameFramework/Character.h"
#include "SpawnVolume/PlayerSpawnVolume.h"
#include "SpawnVolume/AISpawnVolume.h"
#include "EngineUtils.h"
#include "SpawnVolume/ItemSpawnVolume.h"

ACH4GameMode::ACH4GameMode()
{
	GameStateClass = ACH4GameStateBase::StaticClass();
	PlayerStateClass = ACH4PlayerState::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
	DefaultPawnClass = nullptr;

}

void ACH4GameMode::BeginPlay()
{
	Super::BeginPlay();


	// 레벨에 배치된 모든 SpawnVolume Actor 참조 가져오기 추후 AI, 캐릭터 스폰에 사용하기 위한 기초 로직.
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABaseSpawnVolume::StaticClass(), SpawnVolumes);



	
	//GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ACH4GameMode::AssignRoles, 1.f, false);


	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ACH4GameMode::TestAssignRoles8Players, 5.f, false);

	StartItemSpawnTimer();

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

	int32 NumPlayers = GS->PlayerArray.Num();
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

	
	SpawnActors(AIClassesToSpawn, AISpawnRadius);

	UpdateMaxArrests(); //스폰 이후 AI 및 캐릭터 종합 후 최대 체포 가능 횟수 업데이트
	
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

 // AI 캐릭터 및 플레이어 캐릭터 삭제용 로직
 // 만약 로비에서 캐릭터가 움직이며 대기하는 구조라면 불필요한 로직이나, 그렇게 될 시 래그돌, 혹은 사망 애니메이션으로 가사 상태로 캐릭터가 구현되어야 할 듯
	TArray<APawn*> PawnsToDestroy;

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* Pawn = *It;
		if (Pawn && Pawn->HasAuthority())
		{
			PawnsToDestroy.Add(Pawn);
		}
	}
	for (APawn* Pawn : PawnsToDestroy)
	{
		if (Pawn)
		{
			AController* Controller = Pawn->GetController();
			if (Controller)
			{
				Controller->UnPossess();
			}
			Pawn->Destroy();
		}
	}
	//폰을 수집한 후, 수집된 폰을 전부 삭제하는 파트.
	//게임 종료 시에만 동작하는 최종 리셋 파트로 혹시 모를 최적화 문제를 위해 보충

	TArray<APlayerState*> PlayerStatesCopy = GameState->PlayerArray;
	for (int32 i = 0; i < PlayerStatesCopy.Num(); ++i)
	{
		ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PlayerStatesCopy[i]);
		if (TPS)
		{
			TPS->SetPlayerRole(EPlayerRole::Unassigned);
		}
	}
	
	//기존 상단에서 타 파트를 우선 실행한 후, 로비 귀환, 및 결과 위젯 실행을 위해 위치 변경.
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
	ClearItems();
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
		PolicePS->RemainingArrests--;
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


void ACH4GameMode::CheckArrestLimit(ACH4PlayerState* PolicePS) // 직관적인 것을 위해 최대 체포 횟수에서 실패 시 마이너스 카운팅 되는 구조로 수정 중.
{
    if (!PolicePS) return;

    if (PolicePS->RemainingArrests <= 0)
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

    	PolicePS->SetPlayerRole(EPlayerRole::Unassigned);
    }
}


void ACH4GameMode::UpdateMaxArrests()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;

	int32 NumPolice = 0;
	TArray<APlayerState*> PlayerStatesCopy = GameState->PlayerArray;

	for (APlayerState* PS : PlayerStatesCopy)
	{
		if (ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PS))
		{
			if (TPS->PlayerRole == EPlayerRole::Police)
			{
				NumPolice++;
			}
		}
	}
	if (NumPolice == 0) return;

	int32 TotalTargets = GS->SpawnedAI + GS->RemainingThieves;
	float ArrestMultiplier = 0.2f;
	int32 TotalArrestChances = FMath::CeilToInt(TotalTargets * ArrestMultiplier);

	TotalArrestChances = FMath::Max(TotalArrestChances, GS->RemainingThieves);
	int32 MaxArrestsPerPlayer = FMath::Max(1, TotalArrestChances / NumPolice);

	for (APlayerState* PS : PlayerStatesCopy)
	{
		if (ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PS))
		{
			if (TPS->PlayerRole == EPlayerRole::Police)
			{
				TPS->SetMaxArrests(MaxArrestsPerPlayer); // 자동으로 RemainingArrests도 초기화
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("최대 체포 횟수 갱신: %d (경찰 %d명, 타겟 %d명, 계수 %.2f)"),
		MaxArrestsPerPlayer, NumPolice, TotalTargets, ArrestMultiplier);
}

/* 미사용 코드, 추후 정상 동작 확인 시 삭제 필요.
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
void ACH4GameMode::SpawnActors(TArray<TSubclassOf<APawn>> AIClasses, float InAISpawnRadius)
{
    if (!HasAuthority() || SpawnVolumes.Num() == 0) 
    {
    	UE_LOG(LogTemp, Warning, TEXT("스폰 볼륨이 없습니다."));
    	return;
    }
    TArray<ABaseSpawnVolume*> AISpawnVolumes;
    TArray<ABaseSpawnVolume*> PlayerSpawnVolumes;
	
	for (AActor* Actor : SpawnVolumes)
	{
		if (AAISpawnVolume* AISV = Cast<AAISpawnVolume>(Actor))
		{
			AISpawnVolumes.Add(AISV);
		}
		else if (APlayerSpawnVolume* PSV = Cast<APlayerSpawnVolume>(Actor))
		{
			PlayerSpawnVolumes.Add(PSV);
		}
		else if (AItemSpawnVolume* ISV = Cast<AItemSpawnVolume>(Actor))
		{
			ItemSpawnVolumes.Add(ISV);
		}
	}

	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActors: GameState가 없습니다."));
		return;
	}

	if (AISpawnVolumes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnActors: AISpawnVolume이 없습니다."));
		return;
	}

	int32 SpawnedCount = 0;
	int32 AttemptCount = 0;
	int32 MaxAttempts = GS->MaxAISpawn * 10; // 충분히 큰 시도 횟수

	while (SpawnedCount < GS->MaxAISpawn && AttemptCount < MaxAttempts)
	{
		AttemptCount++;

		// 랜덤 볼륨 선택 및 볼륨 내부 랜덤 위치
		int32 VolumeIndex = FMath::RandRange(0, AISpawnVolumes.Num() - 1);
		FVector SpawnLocation = AISpawnVolumes[VolumeIndex]->GetSpawnLocation();

		// 네비게이션에서 랜덤 포인트 찾기
		UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (!NavSys) break;

		FNavLocation RandomPoint;
		if (!NavSys->GetRandomPointInNavigableRadius(SpawnLocation, InAISpawnRadius, RandomPoint))
		{
			UE_LOG(LogTemp, Warning, TEXT("AI 스폰 실패: 네비 포인트 없음. 시도 %d/%d"), AttemptCount, MaxAttempts);
			continue;
		}

		// AI 클래스 랜덤 선택
		int32 ClassIndex = FMath::RandRange(0, AIClasses.Num() - 1);
		TSubclassOf<APawn> AIClass = AIClasses[ClassIndex];
		if (!AIClass) continue;

		// 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* NewAI = GetWorld()->SpawnActor<APawn>(AIClass, RandomPoint.Location, FRotator::ZeroRotator, SpawnParams);
		if (NewAI)
		{
			SpawnedCount++;
			GS->SpawnedAI++;
			UE_LOG(LogTemp, Log, TEXT("AI 스폰됨. 총 AI: %d/%d"), GS->SpawnedAI, GS->MaxAISpawn);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("AI 스폰 완료: %d/%d"), GS->SpawnedAI, GS->MaxAISpawn);
	
	
	//여기서부턴 플레이어 스폰 과정
	if (!GS) return;
	TArray<APlayerState*> Players = GS->PlayerArray;
    Algo::RandomShuffle(PlayerSpawnVolumes);
    Algo::RandomShuffle(Players);

    int32 SpawnCount = FMath::Min(PlayerSpawnVolumes.Num(), Players.Num());
    TArray<APawn*> SpawnedPawns;

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        ACH4PlayerState* TPS = Cast<ACH4PlayerState>(Players[i]);
        if (!TPS) continue;

        FVector SpawnLoc = PlayerSpawnVolumes.IsValidIndex(i) ? PlayerSpawnVolumes[i]->GetActorLocation() : FVector::ZeroVector;
        if (SpawnLoc.IsZero()) continue;

        TSubclassOf<APawn> PawnClassToSpawn = nullptr;
        switch (TPS->PlayerRole)
        {
            case EPlayerRole::Police: PawnClassToSpawn = PolicePawnClass; break;
            case EPlayerRole::Thief:  PawnClassToSpawn = ThiefPawnClass; break;
            default: continue;
        }

        if (!PawnClassToSpawn) continue;

        APawn* PlayerPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, SpawnLoc, FRotator::ZeroRotator);
        if (!PlayerPawn) continue;

        SpawnedPawns.Add(PlayerPawn);

        // Controller 가져오기 및 Possess
        if (AController* PC = TPS->GetOwner<AController>())
        {
            PC->Possess(PlayerPawn);
            UE_LOG(LogTemp, Log, TEXT("%s 스폰 완료 및 Possess 완료"), *TPS->GetPlayerName());
        }

        // 입력 제한
        if (ACharacter* Char = Cast<ACharacter>(PlayerPawn))
        {
            Char->DisableInput(nullptr);
        }
    }

    // 일정 시간 후 입력 제한 해제
    float UnlockDelay = 5.f;
    GetWorldTimerManager().ClearTimer(UnlockPlayerInputTimerHandle);

    GetWorldTimerManager().SetTimer(
        UnlockPlayerInputTimerHandle,
        [SpawnedPawns]()
        {
            for (APawn* SpawnedPawn : SpawnedPawns)
            {
                if (!SpawnedPawn) continue;

                if (ACharacter* Char = Cast<ACharacter>(SpawnedPawn))
                {
                    if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
                    {
                        Char->EnableInput(PC);
                    }
                }
            }
        },
        UnlockDelay,
        false
    );
}


//동작 테스트용 함수
void ACH4GameMode::TestAssignRoles8Players()
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;

	TArray<APlayerState*> PlayerStates = GS->PlayerArray;
	if (PlayerStates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestAssignRoles8Players: 플레이어가 없습니다."));
		return;
	}

	AssignRoles();

	UE_LOG(LogTemp, Log, TEXT("==== TestAssignRoles8Players ===="));
	for (APlayerState* PS : PlayerStates)
	{
		if (ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PS))
		{
			UE_LOG(LogTemp, Log, TEXT("%s 역할: %s"),
				*TPS->GetPlayerName(),
				*UEnum::GetValueAsString(TPS->PlayerRole));
		}
	}
}

// 플레이어가 아이템 박스 겹쳤을 때 서버에서 처리 : 마리오카트처럼, 무작위 아이템
// 추후 플레이어 컨트롤러로 확정될 시 수정 필요
void ACH4GameMode::GivePlayerItem(APlayerController* Player, FName ItemID)
{
	// 서버 권한 확인
	if (!HasAuthority() || !Player) return;

	// 서버에서 GameState 가져오기
	ACH4GameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<ACH4GameStateBase>() : nullptr;
	if (!GS)
	{
		UE_LOG(LogTemp, Warning, TEXT("GivePlayerItem: GameState가 없습니다."));
		return;
	}

	// PlayerState 가져오기
	if (ACH4PlayerState* PS = Player->GetPlayerState<ACH4PlayerState>())
	{
		PS->AddItemToInventory(ItemID);
		UE_LOG(LogTemp, Log, TEXT("Gave Item %s to Player %s"), *ItemID.ToString(), *PS->GetPlayerName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("GivePlayerItem: PlayerState를 가져오지 못했습니다."));
	}
}

//마리오카트처럼 랜덤 아이템 스폰 방식
//서버 내에서만 관리할 스폰 로직으로, 게임 스테이트, 플레이어 스테이트에서 관리하지 않음.
//1분마다 기존 스폰된 아이템 박스를 삭제하고, 새로운 위치에 아이템 박스를 스폰하는 시스템.


void ACH4GameMode::SpawnItems()
{
	if (!HasAuthority()) return;

	// 기존 아이템 삭제
	ClearItems();

	if (ItemSpawnVolumes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnItems: ItemSpawnVolumes가 없습니다."));
		return;
	}

	int32 ItemsToSpawn = MaxItemCount;

	for (int32 i = 0; i < ItemsToSpawn; ++i)
	{
		// 랜덤 볼륨 선택
		int32 VolumeIndex = FMath::RandRange(0, ItemSpawnVolumes.Num() - 1);
		AItemSpawnVolume* Volume = ItemSpawnVolumes[VolumeIndex];
		if (!Volume || Volume->ItemClasses.Num() == 0) continue;

		// 랜덤 아이템 선택
		int32 ClassIndex = FMath::RandRange(0, Volume->ItemClasses.Num() - 1);
		TSubclassOf<AActor> ItemClass = Volume->ItemClasses[ClassIndex];

		FVector SpawnLocation = Volume->GetSpawnLocation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* NewItem = GetWorld()->SpawnActor<AActor>(ItemClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (NewItem)
		{
			NewItem->SetReplicates(true);
			SpawnedItems.Add(NewItem);
			CurrentItemCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("아이템 스폰 완료: %d/%d"), CurrentItemCount, MaxItemCount);
}

/*
//네비매쉬가 존재할 경우만 아이템을 스폰하는 예시 로직.
void ACH4GameMode::SpawnItems()
{
	if (!HasAuthority()) return;

	// 기존 아이템 삭제
	ClearItems();

	if (ItemSpawnVolumes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnItems: ItemSpawnVolumes가 없습니다."));
		return;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnItems: NavigationSystem 없음"));
		return;
	}

	int32 ItemsToSpawn = MaxItemCount;

	for (int32 i = 0; i < ItemsToSpawn; ++i)
	{
		// 랜덤 볼륨 선택
		int32 VolumeIndex = FMath::RandRange(0, ItemSpawnVolumes.Num() - 1);
		AItemSpawnVolume* Volume = ItemSpawnVolumes[VolumeIndex];
		if (!Volume || Volume->ItemClasses.Num() == 0) continue;

		// 랜덤 아이템 선택
		int32 ClassIndex = FMath::RandRange(0, Volume->ItemClasses.Num() - 1);
		TSubclassOf<AActor> ItemClass = Volume->ItemClasses[ClassIndex];

		// 네비게이션 내 랜덤 포인트
		FVector DesiredLocation = Volume->GetSpawnLocation();
		FNavLocation NavLocation;
		bool bFoundNavLocation = NavSys->GetRandomPointInNavigableRadius(DesiredLocation, 200.f, NavLocation);

		if (!bFoundNavLocation)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnItems: 네비 포인트를 찾지 못함"));
			continue;
		}

		// 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* NewItem = GetWorld()->SpawnActor<AActor>(
			ItemClass,
			NavLocation.Location,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (NewItem)
		{
			NewItem->SetReplicates(true);
			SpawnedItems.Add(NewItem);
			CurrentItemCount++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("아이템 스폰 완료: %d/%d"), CurrentItemCount, MaxItemCount);
}

 */


//1분마다 아이템 삭제 후, 리스폰하는 구조.
void ACH4GameMode::StartItemSpawnTimer()
{
	if (!HasAuthority()) return; // 서버 전용

	// 타이머 설정
	GetWorldTimerManager().SetTimer(
		ItemSpawnTimerHandle,
		this,
		&ACH4GameMode::SpawnItems,
		ItemSpawnInterval,
		true // 반복
	);

	// 처음 게임 시작 시 한 번 스폰
	SpawnItems();
}

void ACH4GameMode::ClearItems()
{
	for (AActor* Item : SpawnedItems)
	{
		if (Item)
		{
			Item->Destroy();
		}
	}
	SpawnedItems.Empty();
	CurrentItemCount = 0;
}

