#pragma once

#include "CoreMinimal.h"
#include "Character/CH4Character.h"
#include "ThiefCharacter.generated.h"

UCLASS()
class CH4PROJECT_API AThiefCharacter : public ACH4Character
{
    GENERATED_BODY()

public:
    AThiefCharacter();

    // �������� ������ �� ���������� ����
    UFUNCTION(Server, Reliable)
    void ServerOnCaughtByPolice();
    void ServerOnCaughtByPolice_Implementation();

    // HUD/UI Ŭ���̾�Ʈ�� (�������� ������ ��)
    UFUNCTION(Client, Reliable)
    void ClientOnTrapped();
    void ClientOnTrapped_Implementation();

protected:


};
