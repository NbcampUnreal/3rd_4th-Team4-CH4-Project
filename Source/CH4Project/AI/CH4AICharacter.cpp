#include "CH4AICharacter.h"
#include "CH4AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

ACH4AICharacter::ACH4AICharacter()
{
	AIControllerClass = ACH4AIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	bUseControllerRotationYaw = false;
	Movement->bUseControllerDesiredRotation = false;
	Movement->MaxWalkSpeed = 300.0f;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

}

void ACH4AICharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACH4AICharacter::Arrest()
{
	// 체포 후 AI 캐릭터 삭제
	if (HasAuthority())
	{
	
	Destroy();
	}
}
 
