#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AThiefCharacter();

	// ������ �ݱ�
	void PickupItem(AActor* ItemActor);

	// ������ ��� �Է�
	UFUNCTION()
	void UseItemInput();

	// �������� ������ ��� ó��
	UFUNCTION(Server, Reliable)
	void ServerUseItem();

	// Ŭ���̾�Ʈ ��ü�� ������ ��� �˸�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastUseItem();

	// �������� ü�� ���� ó��
	UFUNCTION(Server, Reliable)
	void Server_TakeDamage(float DamageAmount);

protected:
	// ���� ������ ��� ó��
	void HandleUseItem(AActor* ItemActor);

	// ���� ������ �ִ� ������
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AActor* HeldItem;

	// ������ ��� �� ����
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bUsingItem;

	// ���� ü��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHealth, Category = "Health")
	float CurrentHealth;

	// �ִ� ü��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	// ü�� ���� �� ȣ��
	UFUNCTION()
	void OnRep_CurrentHealth();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
