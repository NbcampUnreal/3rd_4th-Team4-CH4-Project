#include "Item/TrapActor.h"
#include "Components/BoxComponent.h"
#include "Character/CH4Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

ATrapActor::ATrapActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	TrapMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrapMesh"));
	TrapMesh->SetupAttachment(RootComponent);
	TrapMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TrapMesh->SetRelativeLocation(FVector(0.f, 0.f, -300.f));
}

void ATrapActor::BeginPlay()
{
	Super::BeginPlay();

	TrapOwner = GetOwner(); 
	SpawnTime = GetWorld()->GetTimeSeconds();
	
	OnActorBeginOverlap.AddDynamic(this, &ATrapActor::OnOverlap);
}

void ATrapActor::OnOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (!HasAuthority()) return; // 서버에서만 처리

	if (ACH4Character* HitCharacter = Cast<ACH4Character>(OtherActor))
	{
		// 설치자면 무시 시간 체크
		if (HitCharacter == TrapOwner)
		{
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if (CurrentTime - SpawnTime < OwnerIgnoreDuration)
			{
				return; 
			}
		}
		
		HitCharacter->ServerPlayStunAnimation();
		
		if (UCharacterMovementComponent* MoveComp = HitCharacter->GetCharacterMovement())
		{
			MoveComp->DisableMovement();

			// 일정 시간 후 원래 속도로 복구
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(
				TimerHandle,
				[MoveComp]() { MoveComp->SetMovementMode(EMovementMode::MOVE_Walking); },
				StunDuration,
				false
			);

			Destroy();
		}
	}
}
