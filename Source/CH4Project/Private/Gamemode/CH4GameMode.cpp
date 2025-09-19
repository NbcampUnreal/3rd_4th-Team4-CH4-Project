// Fill out your copyright notice in the Description page of Project Settings.


#include "Gamemode/CH4GameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameState/CH4GameStateBase.h"
#include "PlayerState/CH4PlayerState.h"
#include "PlayerController/CH4PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"
#include "NavigationSystem.h"
#include "SpawnVolume/BaseSpawnVolume.h"
#include "SpawnVolume/PlayerSpawnVolume.h"
#include "SpawnVolume/AISpawnVolume.h"
#include "EngineUtils.h"
#include "SpawnVolume/ItemSpawnVolume.h"
#include "IngameUI/CH4UserWidget.h"

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



	
	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ACH4GameMode::AssignRoles, 5.f, false);
	//추후 딜레이 수정 후 실제 플레이어들이 포함되는 테스트가 필요.
	//로비 -> 게임레벨 구조라면 기본 플레이어 컨트롤러가 이미 서버에서 관리 중이기 때문에 완성 후 딜레이를 줄이는 것은 큰 문제는 없을 것으로 추정됨.
	//Only Local Player Controllers can be assigned to widgets. BP_CH4PlayerController_C_0 is not a Local Player Controller. 이런 에러 문구가 뜨며, 추후 플레이어 컨트롤러에서
	//플레이어 컨트롤러에서 if (IsLocalController())를 추가해서 로컬 플레이어만 처리하도록 수정해야함.
	//추가로 기존 5초 딜레이 중 추가적인 인게임 위젯으로 5초간 대기 시간 동안 역할 설명 및 로딩 화면 표현이 필요할 것으로 보임.
	//디폴트 폰 클래스를 추가하면 기본 스폰된 캐릭터들이 플레이어 스타트 지점에 플레이어 수 만큼 캐릭터들이 스폰되어 있기 때문에 초기 시작 화면이 어색함.
 

	//GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &ACH4GameMode::TestAssignRoles8Players, 10.f, false);
	//테스트용 로직에선 플레이어 컨트롤러가 로드된 이후에 스폰해야하기 때문에 10초로 설정

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

	//UpdateMaxArrests(); //스폰 이후 AI 및 캐릭터 종합 후 최대 체포 가능 횟수 업데이트
	
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


//MatchTypes와 WinTeam를 통한 매치 상태 및, 승리 역할군에 따른 UI 작업 필요함.
void ACH4GameMode::CheckWinCondition() //승리 조건 체크 로직으로 추후 필요 시 조건 추가 필요.
{
	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;

	if (GS->RemainingThieves <= 0) //경찰 승리조건
	{
		FinalWinner = EWinTeam::Police;
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
		
	}
	if (GS->MatchTime <= 0.f)
	{
		FinalWinner = EWinTeam::Thief;
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
		
	}
	else if (GS->RemainingPolice <= 0 )
	{
		FinalWinner = EWinTeam::Thief;
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
	}
}

void ACH4GameMode::HandleGameOver()
{
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	//GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	//아이템 스폰 타이머도 초기화 필요.
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(It->Get());
		if (MyPC && MyPC->MyHUDWidget)
		{
			if (MyPC->MyHUDWidget->IsInViewport())
			{
				MyPC->MyHUDWidget->RemoveFromParent();
			}
			MyPC->MyHUDWidget = nullptr;
		}

		//아이템 스폰 타이머 클리어로 추가 아이템 스폰 중지.
		GetWorldTimerManager().ClearTimer(ItemSpawnTimerHandle);

	}

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
		APlayerController* PC = It->Get();
		if (PC)
		{
			PC->ClientTravel(TEXT("/Game/Maps/LobbyMap.umap"), TRAVEL_Absolute);
		}
	}
	
	ClearItems();
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

//삭제 필요 로직
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
	ItemSpawnVolumes.Reset();
	
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

		//AI가 바닥 안에 스폰되는 케이스를 해결하기 위한 지면 보정.
		FVector AdjustedLocation = RandomPoint.Location;
		FHitResult Hit;
		FVector Start = AdjustedLocation + FVector(0, 0, 500.f);
		FVector End   = AdjustedLocation - FVector(0, 0, 500.f);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			AdjustedLocation = Hit.Location;
		}
		

		// AI 클래스 랜덤 선택
		int32 ClassIndex = FMath::RandRange(0, AIClasses.Num() - 1);
		TSubclassOf<APawn> AIClass = AIClasses[ClassIndex];
		if (!AIClass) continue;
		

		// 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		//FVector NavSpawnLoc = RandomPoint.Location;
		//NavSpawnLoc.Z += 50.f;
		APawn* NewAI = GetWorld()->SpawnActor<APawn>(AIClass, AdjustedLocation, FRotator::ZeroRotator, SpawnParams);
		if (NewAI)
		{
			SpawnedCount++;
			GS->SpawnedAI++;
			UE_LOG(LogTemp, Log, TEXT("AI 스폰됨. 총 AI: %d/%d"), GS->SpawnedAI, GS->MaxAISpawn);
		}

#if WITH_EDITOR
		// === 디버그: 스폰 위치 시각화 (에디터에서만) ===
		DrawDebugSphere(GetWorld(), AdjustedLocation, 50.f, 12, FColor::Green, false, 5.f);
#endif
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
    	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    	{
    		APlayerController* PC = It->Get();
    		if (PC && PC->PlayerState == TPS)
    		{
    			PC->Possess(PlayerPawn);
    			UE_LOG(LogTemp, Log, TEXT("%s 스폰 완료 및 Possess 완료"), *TPS->GetPlayerName());
    			break;
    		}
    	}
    	
    }
	
}




//동작 테스트용 함수
void ACH4GameMode::TestAssignRoles8Players()
{
	if (!HasAuthority()) return;

	ACH4GameStateBase* GS = GetGameState<ACH4GameStateBase>();
	if (!GS) return;

	TArray<APlayerState*> PlayerStates = GS->PlayerArray;
	if (PlayerStates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TestAssignRoles8Players: 플레이어가 없습니다."));
		return;
	}

	// 안정적인 Pawn 스폰/포제스 처리
	CheckControllersAndSpawn(PlayerStates);
}


void ACH4GameMode::CheckControllersAndSpawn(const TArray<APlayerState*>& PlayerStates)
{
    bool bAllControllersReady = true;

    int32 Index = 0;
    for (APlayerState* PS : PlayerStates)
    {
        ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PS);
        if (!TPS) continue;

        // PlayerController를 PlayerState 기준으로 찾아오기
        APlayerController* PC = nullptr;
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            APlayerController* TmpPC = It->Get();
            if (TmpPC && TmpPC->PlayerState == TPS)
            {
                PC = TmpPC;
                break;
            }
        }

        if (!PC)
        {
            bAllControllersReady = false;
            continue;
        }

        // Pawn이 없으면 Spawn + Possess
        if (!TPS->GetPawn())
        {
            TSubclassOf<APawn> PawnClassToSpawn = nullptr;
            switch (TPS->PlayerRole)
            {
                case EPlayerRole::Police: PawnClassToSpawn = PolicePawnClass; break;
                case EPlayerRole::Thief:  PawnClassToSpawn = ThiefPawnClass;  break;
                default: continue;
            }
            if (!PawnClassToSpawn) continue;

            FVector SpawnLoc = FVector(200.f * Index, 0.f, 100.f); // 테스트용 위치 분산
            FRotator SpawnRot = FRotator::ZeroRotator;

            APawn* PlayerPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, SpawnLoc, SpawnRot);
            if (!PlayerPawn) continue;

            PlayerPawn->SetReplicates(true);
            PlayerPawn->SetReplicateMovement(true);
            PC->Possess(PlayerPawn);

            UE_LOG(LogTemp, Log, TEXT("%s 스폰 및 Possess 완료"), *TPS->GetPlayerName());
        }

        Index++;
    }

    if (!bAllControllersReady)
    {
        // 모든 컨트롤러가 준비될 때까지 0.2초 후 재시도
        FTimerHandle RetryTimerHandle;
        GetWorldTimerManager().SetTimer(RetryTimerHandle, [this, PlayerStates]()
        {
            CheckControllersAndSpawn(PlayerStates);
        }, 0.2f, false);

        return;
    }

    // 모든 컨트롤러 준비 완료 -> 역할 배정 및 로그
    AssignRoles();

    UE_LOG(LogTemp, Log, TEXT("==== TestAssignRoles8Players ===="));
    for (APlayerState* PS : PlayerStates)
    {
        if (ACH4PlayerState* TPS = Cast<ACH4PlayerState>(PS))
        {
            UE_LOG(LogTemp, Log, TEXT("%s 역할: %s, Pawn: %s"),
                *TPS->GetPlayerName(),
                *UEnum::GetValueAsString(TPS->PlayerRole),
                TPS->GetPawn() ? TEXT("Spawned") : TEXT("None"));
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

//공중에서 스폰되는 케이스 발생으로, 이 로직을 사용할 시 아이템스폰볼륨을 레벨 내에 다수 배치해 두어야할 듯.
/*
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
*/

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
		if (!ItemClass) continue;

		// 네비게이션 내 랜덤 포인트
		FVector DesiredLocation = Volume->GetSpawnLocation();
		FNavLocation NavLocation;
		bool bFoundNavLocation = NavSys->GetRandomPointInNavigableRadius(DesiredLocation, 200.f, NavLocation);

		if (!bFoundNavLocation)
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnItems: 네비 포인트를 찾지 못함"));
			continue;
		}

		// === 지면 보정: NavMesh 좌표에서 LineTrace로 실제 바닥 찾기 ===
		FVector AdjustedLocation = NavLocation.Location;
		FHitResult Hit;
		FVector Start = AdjustedLocation + FVector(0, 0, 500.f);
		FVector End   = AdjustedLocation - FVector(0, 0, 500.f);

		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
		{
			AdjustedLocation = Hit.Location;
		}

		// 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* NewItem = GetWorld()->SpawnActor<AActor>(
			ItemClass,
			AdjustedLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (NewItem)
		{
			NewItem->SetReplicates(true);
			SpawnedItems.Add(NewItem);
			CurrentItemCount++;

#if WITH_EDITOR
			// 디버그용 스폰 위치 확인
			DrawDebugSphere(GetWorld(), AdjustedLocation, 30.f, 12, FColor::Yellow, false, 5.f);
#endif
		}
	}

	UE_LOG(LogTemp, Log, TEXT("아이템 스폰 완료: %d/%d"), CurrentItemCount, MaxItemCount);
}




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

