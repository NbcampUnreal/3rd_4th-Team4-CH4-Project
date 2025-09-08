// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Type/MatchTypes.h"
#include "MainGameState.generated.h"

/**
 * 
 */
UCLASS()
class CH4PROJECT_API AMainGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMainGameState();

	UPROPERTY(ReplicatedUsing=OnRep_MatchTypes, BlueprintReadOnly)
	EMatchTypes MatchTypes;

	void SetMatchState(EMatchTypes NewMatchType);

	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 RemainingThieves;
	
	UPROPERTY(Replicated, BlueprintReadOnly)
	float MatchTime;

	UFUNCTION()
	void OnRep_MatchTypes();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
