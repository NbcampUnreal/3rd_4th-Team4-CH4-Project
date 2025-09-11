// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpawnVolume/BaseSpawnVolume.h"
#include "ItemSpawnVolume.generated.h"

/**
 * 
 */
UCLASS()
class CH4PROJECT_API AItemSpawnVolume : public ABaseSpawnVolume
{
	GENERATED_BODY()

	
public:
	// 스폰할 아이템 클래스 배열
	UPROPERTY(EditAnywhere, Category="Spawn")
	TArray<TSubclassOf<AActor>> ItemClasses;

	// 반경을 기준으로 랜덤 위치 반환
	virtual FVector GetSpawnLocation() const override;
	
};
