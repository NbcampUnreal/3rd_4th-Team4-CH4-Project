#include "Character/ThiefCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AThiefCharacter::AThiefCharacter()
{
	bReplicates = true;
	HeldItem = nullptr;
	bUsingItem = false;

	// 초기 체력 설정
	CurrentHealth = MaxHealth;
}

void AThiefCharacter::PickupItem(AActor* ItemActor)
{
	if (ItemActor)
	{
		HeldItem = ItemActor;
	}
}

void AThiefCharacter::UseItemInput()
{
	if (HeldItem)
	{
		if (HasAuthority())
		{
			HandleUseItem(HeldItem);
		}
		else
		{
			ServerUseItem();
		}
	}
}

void AThiefCharacter::ServerUseItem_Implementation()
{
	HandleUseItem(HeldItem);
}

void AThiefCharacter::MulticastUseItem_Implementation()
{
	// 효과(소리, 이펙트 등) 처리
}

void AThiefCharacter::HandleUseItem(AActor* ItemActor)
{
	if (!ItemActor) return;

	bUsingItem = true;

	// 아이템 태그에 따라 효과 적용
	if (ItemActor->ActorHasTag("SpeedBoostItem"))
	{
		GetCharacterMovement()->MaxWalkSpeed += 300.f;
	}
	else if (ItemActor->ActorHasTag("ClockItem"))
	{
		// 시계 아이템 효과 처리
	}
	else if (ItemActor->ActorHasTag("TrapItem"))
	{
		// 덫 아이템 효과 처리
	}

	HeldItem = nullptr;
	ItemActor->Destroy();

	MulticastUseItem();
}

// 체력 동기화
void AThiefCharacter::OnRep_CurrentHealth()
{
	// UI 업데이트 등
}

// 서버에서 체력 감소 처리
void AThiefCharacter::Server_TakeDamage_Implementation(float DamageAmount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		// 사망 처리
		Destroy();
	}
}

// 네트워크 복제 설정
void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThiefCharacter, bUsingItem);
	DOREPLIFETIME(AThiefCharacter, CurrentHealth);
}
