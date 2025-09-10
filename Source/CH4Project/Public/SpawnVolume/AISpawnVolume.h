// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnVolume/BaseSpawnVolume.h"
#include "AISpawnVolume.generated.h"

/**
 * 
 */
UCLASS()
class CH4PROJECT_API AAISpawnVolume : public ABaseSpawnVolume
{
	GENERATED_BODY()
public:
	virtual FVector GetSpawnLocation() const;
	

};
