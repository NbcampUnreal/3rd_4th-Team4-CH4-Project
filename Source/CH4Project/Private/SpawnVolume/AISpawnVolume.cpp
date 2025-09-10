// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume/AISpawnVolume.h"



FVector AAISpawnVolume::GetSpawnLocation() const
{
	if (!SpawnArea) return FVector::ZeroVector;

	return SpawnArea->Bounds.Origin;
}