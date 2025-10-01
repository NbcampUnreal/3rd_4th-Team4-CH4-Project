// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Type/MatchTypes.h"
#include "CH4GameStateBase.generated.h"

USTRUCT(BlueprintType)
struct FKillFeedEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString KillerName;

	UPROPERTY(BlueprintReadOnly)
	FString VictimName;
};

UCLASS()
class CH4PROJECT_API ACH4GameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	ACH4GameStateBase();

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="AI")
	int32 MaxAISpawn;
	
	UPROPERTY(ReplicatedUsing=OnRep_FinalResult, BlueprintReadOnly, Category="Game")
	EWinTeam FinalResult = EWinTeam::None;
	
	UFUNCTION()
	void OnRep_FinalResult();

	void SetFinalResult(EWinTeam NewResult);


	/** 매치 상태 변경 */
	void SetMatchState(EMatchTypes NewMatchType);

	/** 서버 RPC: 매치 상태 설정 */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetMatchState(EMatchTypes NewMatchType);
	bool ServerSetMatchState_Validate(EMatchTypes NewMatchType);

	// 현재까지 발생한 모든 킬 로그
	UPROPERTY(ReplicatedUsing=OnRep_KillFeed)
	TArray<FKillFeedEntry> KillFeed;

	// 서버에서 킬피드 추가
	UFUNCTION(BlueprintCallable)
	void AddKillFeed(ACH4PlayerState* Killer, ACH4PlayerState* Thief, const FString& VictimOverrideName = TEXT("Citizen"));
	
	UFUNCTION()
	void OnRep_KillFeed();
	
	UFUNCTION()
	void OnRep_MatchTypes();

	UFUNCTION()
	void OnRep_RemainingThieves();

	UFUNCTION()
	void OnRep_RemainingPolice();

	UFUNCTION()
	void OnRep_MatchTime();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FString GetRoleText(EPlayerRole Role) const;

};
