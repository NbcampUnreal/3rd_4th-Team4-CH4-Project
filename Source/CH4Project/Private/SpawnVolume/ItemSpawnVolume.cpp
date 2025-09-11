// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume/ItemSpawnVolume.h"

FVector AItemSpawnVolume::GetSpawnLocation() const
{
	if (!SpawnArea) return FVector::ZeroVector;

	const FVector Origin = SpawnArea->Bounds.Origin;
	const FVector Extent = SpawnArea->Bounds.BoxExtent;

	// Box 범위 내 랜덤 위치
	float RandX = FMath::FRandRange(-Extent.X, Extent.X);
	float RandY = FMath::FRandRange(-Extent.Y, Extent.Y);
	float RandZ = FMath::FRandRange(-Extent.Z, Extent.Z);

	return Origin + FVector(RandX, RandY, RandZ);
}