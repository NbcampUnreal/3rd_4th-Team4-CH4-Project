#include "Item/PickUp.h"
#include "Item/BaseItem.h"
#include "Character/CH4Character.h"

APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMesh;
	StaticMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APickUp::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &APickUp::OnOverlap);
}

void APickUp::OnOverlap(AActor* OverlapActor, AActor* OtherActor)
{
	if (!HasAuthority()) return;

	if (ACH4Character* Character = Cast<ACH4Character>(OtherActor))
	{
		if (ItemClass)
		{
			Destroy();
		}
	}
}
