// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameState/MainGameState.h"
#include "PlayerState/MainPlayerState.h"
#include "Type/MatchTypes.h"
#include "Kismet/GameplayStatics.h"
#include "MainGameMode.generated.h"

/**

경찰, 도둑, 시민 타입과 게임의 진행 상황을 enum에 저장함으로 코드 재사용성을 극대화.
시작 대기, 시작, 종료의 사이클을 관리하기 위한 로직 테스트 필요

추후 추가되야할 로직
플레이어 캐릭터 및, AI 캐릭터 랜덤 스폰 위치 설정.
Replication 처리 필요

AI 및 플레이어 사망 시 구속되거나 캐릭터 삭제 로직 구현 필요.
-사망 시 Destroy를 호출, 만약 체포로 변환할 경우 Type의 파트를 추가해서 관리해야할 필요 있음. (기본으로 Destroy로 구현 예정)

게임 종료 조건이 게임 스테이트에서 만족 시 게임 모드에서 레벨 초기화 및 게임 로비로 전환 로직 (지금은 단판으로 구현 되었음)
-5전 3승제 로직 구현?
-기존 구현 로직 중 승패 관리 파트의 전면 로직 수정 필요.

게임의 승패에 따른 UI 출력 필요
-일정 시간, 혹은 로비로 이동 버튼을 클릭 시 위젯 삭제 후 로비 이동 로직 구현 필요

경찰의 도둑 체포와 체포 중 시민 오인 체포 시 카운팅 및 사직 처리 로직 구현 필요

아이템 스폰 로직 구현 및 지속 스폰 로직을 구현 필요.

+ 구현하면 좋을 것같은 파트
월드 내 감옥을 구현 후 감옥에 갇힌 도둑을 다른 도둑이 구하는 로직 및 구조물 구현 등

 */
UCLASS()
class CH4PROJECT_API AMainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainGameMode();
	
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	
	// 매치 상태 변경
	void SetMatchState(EMatchTypes NewMatchType);

	// 역할 배정
	void AssignRoles();

	// 게임 종료 조건 체크
	void CheckWinCondition();

	//게임 재시작
	void RestartGame();

protected:
	//게임 시작을 위한 타이머
	FTimerHandle GameStartTimerHandle;

	//게임 종료 시 클라이언트 퇴장을 위한 함수 : 추후 로비 완성 시, 로비로 이동 연동 필요 있음.
	void HandleGameOver();
};
