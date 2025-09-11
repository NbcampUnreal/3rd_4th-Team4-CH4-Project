// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume/PlayerSpawnVolume.h"


FVector APlayerSpawnVolume::GetSpawnLocation() const
{
	return SpawnArea->Bounds.Origin; // 항상 중앙 고정
}