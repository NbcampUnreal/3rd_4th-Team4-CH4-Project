#include "Character/PoliceCH4Character.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

#include "Gamemode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "PlayerState/CH4PlayerState.h"

// �� �ʿ� ��: ������ Ŭ����/�������̽� ���

APoliceCH4Character::APoliceCH4Character()
{
    bReplicates = true;
    SetReplicateMovement(true);
}

void APoliceCH4Character::BeginPlay()
{
    Super::BeginPlay();
}

void APoliceCH4Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (ArrestAction)
        {
            EIC->BindAction(ArrestAction, ETriggerEvent::Started, this, &APoliceCH4Character::OnArrestInput);
        }
    }
}

/* ===================== Arrest ===================== */

void APoliceCH4Character::OnArrestInput()
{
    // Ŭ�� �Է� �� ������ ����
    ServerTryArrest();
}

void APoliceCH4Character::ServerTryArrest_Implementation()
{
    AActor* Target = FindArrestTarget(ArrestTraceDistance, ArrestTraceRadius);
    bool bSuccess = false;

    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        // Ÿ���� PlayerState�� ���� Ȯ�� (������ �ù�/AI ���)
        if (ACH4PlayerState* TargetPS = TargetPawn->GetPlayerState<ACH4PlayerState>())
        {
            if (TargetPS->PlayerRole == EPlayerRole::Thief)
            {
                bSuccess = true;

                // GameMode�� ���� ü�� ó�� ����(���� �� ����, Destroy, �¸� ���� ��)
                if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
                {
                    if (APlayerController* MyPC = Cast<APlayerController>(GetController()))
                    {
                        GM->HandleArrest(MyPC, TargetPawn);
                    }
                }
            }
            else if (TargetPS->PlayerRole == EPlayerRole::Police)
            {
                // ������ ü�� ��� �ƴ� �� ���� ó��(����/��� UI)
                bSuccess = false;
            }
            else
            {
                // Unassigned �� �� ���� ���(�ù�/AI�� ����)
                bSuccess = false;

                // �ù� ���� ü���� GameMode ������ �ѵ� ����/���� ���� ���� ��
                if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
                {
                    if (APlayerController* MyPC = Cast<APlayerController>(GetController()))
                    {
                        GM->HandleArrest(MyPC, TargetPawn); // ���ο��� AI�� OnAICaught ���
                    }
                }
            }
        }
        else
        {
            // PlayerState�� ������ AI/�ù� ��� �� GameMode�� ����
            if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
            {
                if (APlayerController* MyPC = Cast<APlayerController>(GetController()))
                {
                    GM->HandleArrest(MyPC, TargetPawn);
                }
            }
            bSuccess = false; // �ù�(����)�� ����
        }
    }

    // ���� ��ε�ĳ��Ʈ
    MulticastPlayArrestFX(bSuccess);

    // ü���� ���ο��Ը� ��� UI
    ClientShowArrestResultUI(bSuccess);
}

void APoliceCH4Character::MulticastPlayArrestFX_Implementation(bool bSuccess)
{
    // TODO: ����/���п� ���� ����Ʈ/����/�ִ�
    UE_LOG(LogTemp, Log, TEXT("[ArrestFX] %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"));
}

void APoliceCH4Character::ClientShowArrestResultUI_Implementation(bool bSuccess)
{
    // TODO: �������Ʈ ���� ����
    // ex) ����: ������ ü��!��, ����: ������ ü�� ���
}

/* ================= Trace Helper ================= */

AActor* APoliceCH4Character::FindArrestTarget(float TraceDistance, float Radius) const
{
    const FVector Start = GetActorLocation() + FVector(0, 0, 50.f);
    const FVector End = Start + GetActorForwardVector() * TraceDistance;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ArrestTrace), false, this);

    // Pawn ä�ο��� Block �ǵ��� ������ ���� �ʿ�
    GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity, ECC_Pawn, Sphere, Params);

    AActor* Best = nullptr;
    float BestDistSq = TNumericLimits<float>::Max();

    for (const FHitResult& H : Hits)
    {
        AActor* A = H.GetActor();
        if (!A || A == this) continue;

        const float DistSq = FVector::DistSquared(Start, A->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Best = A;
        }
    }

#if !(UE_BUILD_SHIPPING)
    DrawDebugLine(GetWorld(), Start, End, FColor::Blue, false, 1.f, 0, 1.5f);
    DrawDebugSphere(GetWorld(), End, Radius, 16, FColor::Cyan, false, 1.f);
#endif

    return Best;
}




