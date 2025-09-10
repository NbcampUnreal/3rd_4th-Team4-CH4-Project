#include "Character/ThiefCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

AThiefCharacter::AThiefCharacter()
{
	bReplicates = true;
	HeldItem = nullptr;
	bUsingItem = false;

	// �ʱ� ü�� ����
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
	// ȿ��(�Ҹ�, ����Ʈ ��) ó��
}

void AThiefCharacter::HandleUseItem(AActor* ItemActor)
{
	if (!ItemActor) return;

	bUsingItem = true;

	// ������ �±׿� ���� ȿ�� ����
	if (ItemActor->ActorHasTag("SpeedBoostItem"))
	{
		GetCharacterMovement()->MaxWalkSpeed += 300.f;
	}
	else if (ItemActor->ActorHasTag("ClockItem"))
	{
		// �ð� ������ ȿ�� ó��
	}
	else if (ItemActor->ActorHasTag("TrapItem"))
	{
		// �� ������ ȿ�� ó��
	}

	HeldItem = nullptr;
	ItemActor->Destroy();

	MulticastUseItem();
}

// ü�� ����ȭ
void AThiefCharacter::OnRep_CurrentHealth()
{
	// UI ������Ʈ ��
}

// �������� ü�� ���� ó��
void AThiefCharacter::Server_TakeDamage_Implementation(float DamageAmount)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		// ��� ó��
		Destroy();
	}
}

// ��Ʈ��ũ ���� ����
void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AThiefCharacter, bUsingItem);
	DOREPLIFETIME(AThiefCharacter, CurrentHealth);
}
