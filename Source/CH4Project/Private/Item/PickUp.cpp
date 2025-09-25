#include "Item/PickUp.h"
#include "Item/BaseItem.h"
#include "Character/CH4Character.h"

APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = StaticMesh;
	StaticMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void APickUp::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &APickUp::OnOverlap);
	
	StartLocation = GetActorLocation();
	RunningTime = 0.f;
}

void APickUp::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsNetMode(NM_DedicatedServer)) return; // 서버는 안 움직임
	// 회전
	AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaSeconds, 0.f));

	// 위아래 이동
	RunningTime += DeltaSeconds;
	float OffsetZ = FMath::Sin(RunningTime * BobSpeed) * BobHeight;

	FVector NewLocation = StartLocation;
	NewLocation.Z += OffsetZ;
	SetActorLocation(NewLocation);
}

void APickUp::OnOverlap(AActor* OverlapActor, AActor* OtherActor)
{
	if (!HasAuthority()) return;

	if (ACH4Character* Character = Cast<ACH4Character>(OtherActor))
	{
		if (ItemClass)
		{
			UBaseItem* NewItem = NewObject<UBaseItem>(Character, ItemClass);
			//Character->AddToInventory(NewItem);
			Destroy();
		}
	}
}