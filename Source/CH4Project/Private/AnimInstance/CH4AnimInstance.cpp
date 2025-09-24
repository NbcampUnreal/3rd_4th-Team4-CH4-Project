#include "AnimInstance/CH4AnimInstance.h"

UCH4AnimInstance::UCH4AnimInstance()
{
    bIsJumping = false;
    bIsRunning = false;
    Speed = 0.f;
    bUsingItem = false;
}

void UCH4AnimInstance::PlayDeadAnimation()
{
    bIsDead = true; 

    if (DeathMontage)
    {
        Montage_Play(DeathMontage);
    }
}
