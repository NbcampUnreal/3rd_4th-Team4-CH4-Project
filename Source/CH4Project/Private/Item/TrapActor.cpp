#include "Item/TrapActor.h"
#include "Components/BoxComponent.h"
#include "Character/CH4Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ATrapActor::ATrapActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	TrapMesh->SetupAttachment(RootComponent);
	TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATrapActor::BeginPlay()
{
	Super::BeginPlay();

	OnActorBeginOverlap.AddDynamic(this, &ATrapActor::OnOverlap);
}

void ATrapActor::OnOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	ACH4Character* HitCharacter = Cast<ACH4Character>(OtherActor);
	if (HitCharacter && !HasAuthority()) return; // 서버에서만 처리

	if (HitCharacter)
	{
		UCharacterMovementComponent* MoveComp = HitCharacter->GetCharacterMovement();
		if (MoveComp)
		{
			// 움직임 정지
			MoveComp->DisableMovement();

			// 타이머로 복구
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[MoveComp]()
				{
					MoveComp->SetMovementMode(EMovementMode::MOVE_Walking);
				},
				StunDuration,
				false
			);
			
			Destroy();
		}
	}
}
