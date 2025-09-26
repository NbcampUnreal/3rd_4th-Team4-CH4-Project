#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CH4ChatPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyStateChanged, bool, bIsReady);

UCLASS()
class CH4PROJECT_API ACH4ChatPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ACH4ChatPlayerState();

    UFUNCTION(BlueprintCallable, Category = "Lobby")
    bool IsReady() const { return bIsReady; }

    UFUNCTION(Server, Reliable, WithValidation)
    void ServerSetReady(bool bNewReady);

    UPROPERTY(BlueprintAssignable, Category = "Lobby")
    FOnReadyStateChanged OnReadyStateChanged;

protected:
    UPROPERTY(ReplicatedUsing = OnRep_IsReady, BlueprintReadOnly, Category = "Lobby")
    bool bIsReady = false;

    UFUNCTION()
    void OnRep_IsReady();


    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
