// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Type/MatchTypes.h"
#include "Kismet/GameplayStatics.h"
#include "SpawnVolume/BaseSpawnVolume.h"
#include "CH4GameMode.generated.h"


class ACH4PlayerState;
class ACH4GameStateBase;
class APawn;

/**

경찰, 도둑, 시민 타입과 게임의 진행 상황을 enum에 저장함으로 코드 재사용성을 극대화.
시작 대기, 시작, 종료의 사이클을 관리하기 위한 로직 테스트 필요

추후 추가되야할 로직
플레이어 캐릭터 및, AI 캐릭터 랜덤 스폰 위치 설정.
-Replication 처리 필요
-경찰과 도둑을 랜덤 위치에 랜덤 스폰하는 방식으로 구현 준비 중.
-SpawnVolume을 통한 플레이어 캐릭터와 AI 캐릭터의 스폰을 나눠 진행할 예정.
-시작 시 5초에서 10초, 15초 단위로 조정 후 대기 시간동안 AI 캐릭터 스폰 로직 등 방안 모색 중.

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

아이템 스폰 로직 구현 및 지속 스폰 로직을 구현 필요.
-이 파트는 아이템 파트가 어느정도 구현된 후에 실행하거나, 3주차에 진행할 예정.



 */
UCLASS()
class CH4PROJECT_API ACH4GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACH4GameMode();
	
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 역할 배정 */
	void AssignRoles();

	/** 매치 시간 업데이트 */
	void UpdateMatchTime();

	/** 체포 처리 */
	void OnThiefCaught(APawn* ThiefPawn);
	void OnAICaught(APlayerController* ArrestingPlayer, APawn* AI, bool bIsCitizen);
	void HandleArrest(APlayerController* ArrestingPlayer, APawn* TargetPawn);

	/** 최대 체포 횟수 갱신 */
	void UpdateMaxArrests();

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
	//캐릭터 스폰 로직 통합 및 개선 테스트 버전
	void SpawnActors(TArray<TSubclassOf<APawn>> AIClasses, float AISpawnRadius);

	
	//실제 동작하는지 테스트 용 함수
	void TestAssignRoles8Players();

protected:
	/*
	//BP 게임모드에서 스폰할 AI BP 캐릭터를 에디터 상으로 선정해야함.
	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	TArray<TSubclassOf<APawn>> AIClassesToSpawn;

	// AI 스폰 반경 (에디터에서 조정 가능)
	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	float AISpawnRadius = 500.f;
	*/
private:
	/** 게임 시작 타이머 */
	FTimerHandle GameStartTimerHandle;

	/** 매치 타이머 */
	FTimerHandle MatchTimerHandle;

	/** 레벨에 배치된 스폰 볼륨 */
	UPROPERTY()
	TArray<AActor*> SpawnVolumes;

	/** 도전 횟수 체크 */
	void CheckArrestLimit(ACH4PlayerState* PolicePS);
};

