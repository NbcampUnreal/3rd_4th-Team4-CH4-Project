#include "Character/ThiefCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Item/BaseItem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AThiefCharacter::AThiefCharacter()
{
   
}

// Ŭ���̾�Ʈ UI ó��
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed();
}

void AThiefCharacter::OnItemUsed()
{
}
