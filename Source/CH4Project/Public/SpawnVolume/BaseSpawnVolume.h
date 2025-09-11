// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BaseSpawnVolume.generated.h"


UCLASS()

class CH4PROJECT_API ABaseSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ABaseSpawnVolume();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Spawn")
	UBoxComponent* SpawnArea;

public:
	virtual FVector GetSpawnLocation() const;
};
