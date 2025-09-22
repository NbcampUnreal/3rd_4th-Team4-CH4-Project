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

/**

AI 및 플레이어 사망 시 구속되거나 캐릭터 삭제 로직 구현 필요.
-사망 시 Destroy를 호출, 만약 체포로 변환할 경우 Type의 파트를 추가해서 관리해야할 필요 있음. (기본으로 Destroy로 구현 예정)
-OnThiefCaught를 통한 도둑 체포 시 로직
-OnAICaught를 통한 시민 오인 체포 시 로직
-HandleArrest로 위 두 로직을 통합 호출하는 방식으로 관리 중.

게임 종료 조건이 게임 스테이트에서 만족 시 게임 모드에서 레벨 초기화 및 게임 로비로 전환 로직 (지금은 단판으로 구현 되었음)
-일반적으로 로비로 전환하는 단판제 시스템이 적합할 것으로 보임.
-레벨 내의 모든 폰을 Destroy 처리한 후 레벨을 새로 로드할 수 있도록 로비로 전환, 이후 로비 위젯을 재출력하는 구조가 좋을 듯.

게임의 승패에 따른 UI 출력 필요
-일정 시간, 혹은 로비로 이동 버튼을 클릭 시 위젯 삭제 후 로비 이동 로직 구현 필요

경찰의 도둑 체포와 체포 중 시민 오인 체포 시 카운팅 및 사직 처리 로직 구현 필요
-CheckArrestLimit와 UpdateMaxArrests 를 통한 AI 및 캐릭터 비례 시도 횟수 동적화 로직 구현
-추후 테스트 필요


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
	void OnThiefCaught(APawn* ThiefPawn);
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



	void GivePlayerItem(APlayerController* Player, FName ItemID);
	
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

