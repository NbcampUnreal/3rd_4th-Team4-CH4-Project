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

// �������� ������ �� ���������� ����
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    // ���� ��带 �����ͼ� ĳ���͸� �ı�
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        Destroy();

        // ���� ���¸� �����ͼ� ���� ���� ���� ����
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            // OnRep �Լ��� �������� ȣ���Ͽ� ���� ���� �� ������ ��� Ŭ���̾�Ʈ�� �˷���
            GS->OnRep_RemainingThieves();
        }

        // Ŭ���̾�Ʈ���� �����ٴ� UI�� �����ֵ��� ��û
        ClientOnTrapped();
    }
}

// Ŭ���̾�Ʈ UI ó��
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed(nullptr);
}
