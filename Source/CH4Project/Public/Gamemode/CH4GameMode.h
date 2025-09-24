// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Type/MatchTypes.h"
#include "SpawnVolume/ItemSpawnVolume.h"
#include "CH4GameMode.generated.h"


class ACH4PlayerState;
class ACH4GameStateBase;
class APawn;
class UBaseItem;

/*

 */
UCLASS()
class CH4PROJECT_API ACH4GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACH4GameMode();
	
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;


	EWinTeam FinalWinner = EWinTeam::None;

	
	/** 역할 배정 */
	void AssignRoles();

	/** 매치 시간 업데이트 */
	void UpdateMatchTime();

	/** 체포 처리 */
	void OnThiefCaught(APawn* ThiefPawn, APlayerController* ArrestingPlayer);
	void OnAICaught(APlayerController* ArrestingPlayer, APawn* AI, bool bIsCitizen);
	void HandleArrest(APlayerController* ArrestingPlayer, APawn* TargetPawn);

	/* 최대 체포 횟수 갱신 */
	//void UpdateMaxArrests();

	/** 승리 조건 체크 */
	void CheckWinCondition();

	/** 게임 오버 처리 */
	void HandleGameOver();
	void RestartGame();

	/** 매치 상태 설정 */
	void SetMatchState(EMatchTypes NewMatchType);

	/*
	void SpawnAI(TSubclassOf<APawn> AIPawnClass, float Radius);

	FVector GetRandomSpawnLocation(float Radius);
	*/

	//로그인 시 플레이어 스테이트로 총 인원을 체크한 후 AssingRoles 콜하는 함수
	void TryAssignRoles();


	
	//캐릭터 스폰 로직 통합 및 개선 테스트 버전
	void SpawnActors(TArray<TSubclassOf<APawn>> AIClasses, float AISpawnRadius);
	
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spawning")
	TArray<AItemSpawnVolume*> ItemSpawnVolumes;
protected:
	
	//BP 게임모드에서 스폰할 각각의 캐릭터를 에디터 상으로 선정해야함.
	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TArray<TSubclassOf<APawn>> AIClassesToSpawn;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawning|Player")
	TSubclassOf<APawn> PolicePawnClass;

	UPROPERTY(EditDefaultsOnly, Category="Spawning|Player")
	TSubclassOf<APawn> ThiefPawnClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	float AISpawnRadius = 500.f;
	
	//모든 플레이어가 빙의한 후, 실제 게임 플레이 카운트다운 타이머
	FTimerHandle UnlockPlayerInputTimerHandle;
	
private:

	bool bRolesAssigned = false;
	int32 ExpectedNumPlayers = 0;
	
	/** 게임 시작 타이머 */
	FTimerHandle GameStartTimerHandle;

	/** 매치 타이머 */
	FTimerHandle MatchTimerHandle;


	/** 레벨에 배치된 스폰 볼륨 */
	UPROPERTY()
	TArray<AActor*> SpawnVolumes;

	/** 도전 횟수 체크 */
	void CheckArrestLimit(ACH4PlayerState* PolicePS);
	
	
	//아이템 스폰 관리용 프라이빗
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Spawning")
	int32 MaxItemCount = 20;

	// 현재 스폰된 아이템 수 (서버 전용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Spawning")
	int32 CurrentItemCount = 0;

	// 주기적 아이템 스폰 타이머
	FTimerHandle ItemSpawnTimerHandle;

	// 주기적 스폰 간격 (초)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Item Spawning")
	float ItemSpawnInterval = 20.f;

	// 맵에 스폰된 아이템 액터들
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item Spawning")
	TArray<AActor*> SpawnedItems;

	// 기존 아이템 삭제
	void ClearItems();

	//아이템 스폰 로직
	void SpawnItems();

	//시작 시 호출하는 아이템 스폰 타이머
	void StartItemSpawnTimer();

	
};

