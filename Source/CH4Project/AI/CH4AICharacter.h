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

	void Arrest(APlayerController* ArrestingPlayer);

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerHandleArrest(APlayerController* ArrestingPlayer);

	void HandleArrestOnServer(APlayerController* ArrestingPlayer);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayArrestMontage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UAnimMontage* ArrestMontage;
};
