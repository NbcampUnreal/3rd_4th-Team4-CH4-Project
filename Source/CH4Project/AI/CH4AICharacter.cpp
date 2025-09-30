#include "CH4AICharacter.h"
#include "CH4AIController.h"
#include "CH4Project/Public/Gamemode/CH4GameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"

ACH4AICharacter::ACH4AICharacter()
{
	bReplicates = true;              // 네트워크 복제
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetIsReplicated(true);
	}
	
	AIControllerClass = ACH4AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	bUseControllerRotationYaw = false;
	Movement->bUseControllerDesiredRotation = false;
	Movement->MaxWalkSpeed = 500.0f;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

}

void ACH4AICharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACH4AICharacter::Arrest(APlayerController* ArrestingPlayer)
{
	if (!ArrestingPlayer) return;
	
	if (HasAuthority())
	{
		HandleArrestOnServer(ArrestingPlayer);
	}
	else
	{
		ServerHandleArrest(ArrestingPlayer);
	}
	
}

void ACH4AICharacter::HandleArrestOnServer(APlayerController* ArrestingPlayer)
{
	if (!ArrestingPlayer) return;

	ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this));
	if (GM)
	{
		GM->HandleArrest(ArrestingPlayer, this);
	}

	MulticastPlayArrestMontage();
}

void ACH4AICharacter::ServerHandleArrest_Implementation(APlayerController* ArrestingPlayer)
{
	if (!ArrestingPlayer) return;

	HandleArrestOnServer(ArrestingPlayer);
}

bool ACH4AICharacter::ServerHandleArrest_Validate(APlayerController* ArrestingPlayer)
{
	return ArrestingPlayer != nullptr;
}

void ACH4AICharacter::MulticastPlayArrestMontage_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Multicast reached: %s"), *GetName());

	if (ArrestMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			AnimInstance->Montage_Play(ArrestMontage);
			UE_LOG(LogTemp, Warning, TEXT("AI %s plays Arrest Montage on all clients"), *GetName());
		}
		
	}
}