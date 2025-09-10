// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Type/MatchTypes.h"
#include "MainGameState.generated.h"

UCLASS()
class CH4PROJECT_API AMainGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMainGameState();

	/** 매치 상태 */
	UPROPERTY(ReplicatedUsing=OnRep_MatchTypes, BlueprintReadOnly)
	EMatchTypes MatchTypes;

	/** 남은 도둑 수 */
	UPROPERTY(ReplicatedUsing=OnRep_RemainingThieves, BlueprintReadOnly)
	int32 RemainingThieves;

	/** 남은 경찰 수 */
	UPROPERTY(ReplicatedUsing=OnRep_RemainingPolice, BlueprintReadOnly)
	int32 RemainingPolice;

	/** 매치 남은 시간 */
	UPROPERTY(ReplicatedUsing=OnRep_MatchTime, BlueprintReadOnly)
	float MatchTime;

	/** 생성된 AI 수 */
	UPROPERTY(Replicated)
	int32 SpawnedAI;

	/** 최대 AI 생성 수 */
	UPROPERTY(Replicated)
	int32 MaxAISpawn;

	/** 전체 플레이어 수 */
	UPROPERTY(Replicated)
	int32 TotalPlayers; //반드시 로비 파트에서 플레이어 총합을 계산해서 이 변수에 저장할 것.*

	/** 매치 상태 변경 */
	void SetMatchState(EMatchTypes NewMatchType);

	/** 서버 RPC: 매치 상태 설정 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetMatchState(EMatchTypes NewMatchType);
	void ServerSetMatchState_Implementation(EMatchTypes NewMatchType);
	bool ServerSetMatchState_Validate(EMatchTypes NewMatchType);

	UFUNCTION()
	void OnRep_MatchTypes();

	UFUNCTION()
	void OnRep_RemainingThieves();

	UFUNCTION()
	void OnRep_RemainingPolice();

	UFUNCTION()
	void OnRep_MatchTime();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
