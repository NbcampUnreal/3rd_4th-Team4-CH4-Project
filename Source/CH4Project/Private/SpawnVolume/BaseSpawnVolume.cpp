//프로젝트 내에서 스폰되는 다양한 액터들의 스폰을 위한 베이스 구성도.
//추후 캐릭터 스폰볼륨, AI 스폰볼륨, 아이템들의 스폰 볼륨 등을 구상 중.

#include "SpawnVolume/BaseSpawnVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

ABaseSpawnVolume::ABaseSpawnVolume()
{
	SpawnArea = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnArea"));
	RootComponent = SpawnArea;
}

FVector ABaseSpawnVolume::GetSpawnLocation() const
{
	return SpawnArea->Bounds.Origin; // 기본은 단순 중앙

}