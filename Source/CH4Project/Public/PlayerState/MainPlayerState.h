// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Type/MatchTypes.h"
#include "MainPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class CH4PROJECT_API AMainPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	AMainPlayerState();

    UPROPERTY(Replicated, BlueprintReadOnly)
	EPlayerRole PlayerRole;

	void SetPlayerRole(EPlayerRole NewRole);
	
	EPlayerRole GetPlayerRole() const { return PlayerRole; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
