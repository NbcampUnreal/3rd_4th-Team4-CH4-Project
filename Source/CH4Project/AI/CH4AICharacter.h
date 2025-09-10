#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CH4AICharacter.generated.h"

UCLASS()
class CH4PROJECT_API ACH4AICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACH4AICharacter();

protected:
	virtual void BeginPlay() override;

	void Arrest();
};
