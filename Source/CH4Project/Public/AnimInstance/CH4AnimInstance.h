#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CH4AnimInstance.generated.h"


UCLASS()
class CH4PROJECT_API UCH4AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    // 캐릭터 상태 변수
    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsJumping;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsRunning;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bIsStunned = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage;

    // 아이템 사용
    UPROPERTY(BlueprintReadWrite, Category = "Animation")
    bool bUsingItem;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsDead;

    UFUNCTION(BlueprintCallable)
	void PlayDeadAnimation();
    
    UCH4AnimInstance();
};
