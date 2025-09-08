// Fill out your copyright notice in the Description page of Project Settings.


#include "Gamemode/MainGameMode.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "GameState/MainGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"
#include "PlayerState/MainPlayerState.h"

AMainGameMode::AMainGameMode()
{
	GameStateClass = AMainGameState::StaticClass();
	PlayerStateClass = AMainPlayerState::StaticClass();
	PrimaryActorTick.bCanEverTick = true;
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	//로비에서 넘어온 후 5초 후 게임 시작을 위한 타이머
    GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AMainGameMode::AssignRoles, 5.0f, false);
}

void AMainGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AMainGameState* GS = GetGameState<AMainGameState>();
	if (GS && GS->MatchTypes == EMatchTypes::InProgress)
	{
		GS->MatchTime -= DeltaSeconds;
		if (GS->MatchTime < 0.f)
		{
			GS->MatchTime = 0.f; // 0으로 고정
		}

	}
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
	TArray<APlayerState*> PlayerStates = GameState->PlayerArray;
	int32 NumPlayers = PlayerStates.Num();
	
	if (NumPlayers == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("AssignRoles: 플레이어가 없습니다."));
		return;
	}

	int32 NumPolice = FMath::Max(1, NumPlayers / 4); //플레이어 수에 비례한 경찰 배정 로직
	int32 NumThieves = NumPlayers - NumPolice;  //전체 플레이어 수에서 폴리스 숫자만큼 뺀 후 도둑을 선정

	int32 PoliceAssigned = 0;

	
	Algo::RandomShuffle(PlayerStates); //플레이어 배열을 랜덤으로 섞는 로직

	for (APlayerState* PS : PlayerStates) // 실제 역할군 선정을 위한 코드.
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(PS);
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

	// 추후 AI 시민 스폰 로직을 추가해야함.

	SetMatchState(EMatchTypes::InProgress);

	//디버그 로그
	UE_LOG(LogTemp, Log, TEXT("경찰 %d명, 도둑 %d명 배정 완료."), NumPolice, NumThieves);

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
		return;
	}
	if (GS->MatchTime <= 0.f)
	{
		SetMatchState(EMatchTypes::GameOver);
		HandleGameOver();
		RestartGame();
		return;
	}
}

void AMainGameMode::HandleGameOver()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It -> Get();

		if (PC && !PC -> IsLocalController())
		{
			PC->ClientTravel(TEXT("/Game/Maps/Lobby"), TRAVEL_Absolute); //로비 맵으로 돌아가는 로직 추후 실제 레벨 경로로 수정 필요.
			//로비 맵으로 돌아간 후 로비 UI 재출력 및(UI 출력하는 로직을 로비 쪽 게임모드 로직으로 대체 가능
			//게임 레벨로 다시 실행되는 구조 작성(이건 로비 쪽 게임모드를 작성하게 되면 그쪽에 보충하는 것이 좋을 듯)
		}
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
	for (APlayerState* PS : GameState->PlayerArray)
	{
		AMainPlayerState* TPS = Cast<AMainPlayerState>(PS);
		if (TPS)
		{
			TPS->SetPlayerRole(EPlayerRole::Unassigned); //부여된 캐릭터의 역할 초기화
			// 점수, 체력 등 초기화 추가 가능
		}
	}
	
}

void AMainGameMode::RestartGame()
{
	AMainGameState* GS = GetGameState<AMainGameState>();
	if (GS)
	{
		GS->MatchTime = 600.f; // 초기값으로 리셋
		GS->RemainingThieves = 2;/* 초기 도둑 수 */;
		SetMatchState(EMatchTypes::WaitingToStart);
	}

	// 필요한 경우 플레이어 상태 초기화, AI 리스폰 등 추가
}




