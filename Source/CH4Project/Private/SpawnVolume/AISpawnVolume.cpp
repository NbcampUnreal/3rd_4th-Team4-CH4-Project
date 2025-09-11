// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnVolume/AISpawnVolume.h"
#include "Kismet/KismetMathLibrary.h"


FVector AAISpawnVolume::GetSpawnLocation() const
{
	if (!SpawnArea) return FVector::ZeroVector;

	FVector Origin = SpawnArea->Bounds.Origin;
	FVector Extent = SpawnArea->Bounds.BoxExtent;

	return UKismetMathLibrary::RandomPointInBoundingBox(Origin, Extent);
}
