#include "Character/PoliceCH4Character.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimInstance.h"
#include "GameMode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "PlayerState/CH4PlayerState.h"

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

/* ��� �� �� ƽ: XY �ӵ�/�Է� ���� + XY�� ��Ŀ�� ���� (Z�� �״�� �� �ڿ� �ϰ�) */
void APoliceCH4Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bMovementLocked)
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            // �ӵ����� XY ���� (���� Z�� ����)
            FVector V = Move->Velocity;
            V.X = 0.f;
            V.Y = 0.f;
            Move->Velocity = V;

            // �����ִ� �Է� ����
            ConsumeMovementInputVector();

            // XY ��ġ�� ��� ���� �������� ���� (Z�� ���簪 ����)
            const FVector Cur = GetActorLocation();
            if (!FMath::IsNearlyEqual(Cur.X, LockAnchorXY.X, 0.01f) ||
                !FMath::IsNearlyEqual(Cur.Y, LockAnchorXY.Y, 0.01f))
            {
                SetActorLocation(FVector(LockAnchorXY.X, LockAnchorXY.Y, Cur.Z), true);
            }
        }
    }
}

/* ===================== Arrest ===================== */

void APoliceCH4Character::OnArrestInput()
{
    if (!IsLocallyControlled()) return;
    if (bArrestOnCooldown_Local) return;

    PlayLocalArrestMontage();
    SetMovementLocked(true);          // ���� ��� ���

    ServerTryArrest();
    StartArrestCooldown_Local();
}

void APoliceCH4Character::ServerTryArrest_Implementation()
{
    APlayerController* MyPC = Cast<APlayerController>(GetController());
    if (!MyPC) return;
    if (bArrestOnCooldown_Server) return;
    StartArrestCooldown_Server();

    // ������ ��� (���� �� �ϰ���)
    SetMovementLocked(true);

    AActor* Target = FindArrestTarget(ArrestTraceDistance, ArrestTraceRadius);
    bool bSuccess = false;

    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        if (ACH4PlayerState* TargetPS = TargetPawn->GetPlayerState<ACH4PlayerState>())
        {
            bSuccess = (TargetPS->PlayerRole == EPlayerRole::Thief);
        }

        if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
        {
            GM->HandleArrest(MyPC, TargetPawn);
        }

        MulticastPlayArrestMontage();
    }
    else
    {
        MulticastPlayArrestMontage();
    }

    MulticastPlayArrestFX(bSuccess);
    ClientShowArrestResultUI(bSuccess);
}

/* ====== ��Ÿ�� ��� ====== */

void APoliceCH4Character::PlayLocalArrestMontage()
{
    if (!ArrestMontage) return;

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
        {
            if (!Anim->Montage_IsPlaying(ArrestMontage))
            {
                Anim->Montage_Play(ArrestMontage, 1.0f);

                // ���� ��������Ʈ: �ߺ� ���� �� ���ε�
                Anim->OnMontageEnded.RemoveDynamic(this, &APoliceCH4Character::OnArrestMontageEnded);
                Anim->OnMontageEnded.AddDynamic(this, &APoliceCH4Character::OnArrestMontageEnded);
            }
        }
    }
}

void APoliceCH4Character::MulticastPlayArrestMontage_Implementation()
{
    PlayLocalArrestMontage();
}

void APoliceCH4Character::MulticastPlayArrestFX_Implementation(bool bSuccess)
{
    UE_LOG(LogTemp, Log, TEXT("[ArrestFX] %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"));
}

void APoliceCH4Character::ClientShowArrestResultUI_Implementation(bool bSuccess)
{
    // TODO: ���� ����
}

/* ============== �̵� ���/���� (�߷� ����, ��Ʈ��� ����, XY ��Ŀ) ============== */

void APoliceCH4Character::SetMovementLocked(bool bLocked)
{
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        if (bLocked)
        {
            if (bMovementLocked) return;
            bMovementLocked = true;

            // ������ ����
            SavedMovementMode = Move->MovementMode;
            SavedAirControl = Move->AirControl;
            SavedOrientToMove = Move->bOrientRotationToMovement;
            SavedUseCtrlYaw = bUseControllerRotationYaw;

            // ��Ʈ��� ���� ����
            if (UAnimInstance* Anim = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
            {
                Anim->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
            }

            // XY �ӵ��� ����(���� Z ���� �� ������ �������� ����)
            FVector V = Move->Velocity;
            V.X = 0.f;
            V.Y = 0.f;
            Move->Velocity = V;

            // ȸ��/���� ����
            Move->AirControl = 0.f;
            Move->bOrientRotationToMovement = false;
            bUseControllerRotationYaw = false;

            // �߷� ����(�����̸� ��� ź��, �����̸� �״�� ����)
            Move->SetMovementMode(MOVE_Falling);
            bPressedJump = false;

            // XY ��Ŀ ����
            const FVector L = GetActorLocation();
            LockAnchorXY = FVector2D(L.X, L.Y);

            // �����ִ� �Է� ����
            ConsumeMovementInputVector();
        }
        else
        {
            if (!bMovementLocked) return;
            bMovementLocked = false;

            // ����
            Move->AirControl = SavedAirControl;
            Move->bOrientRotationToMovement = SavedOrientToMove;
            bUseControllerRotationYaw = SavedUseCtrlYaw;

            Move->SetMovementMode(MOVE_Walking);

            // ��Ʈ��� ��� �⺻�� ����
            if (UAnimInstance* Anim = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
            {
                Anim->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
            }
        }
    }
}

void APoliceCH4Character::OnArrestMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == ArrestMontage)
    {
        SetMovementLocked(false);
    }
}

/* ================= Trace Helper ================= */

AActor* APoliceCH4Character::FindArrestTarget(float TraceDistance, float Radius) const
{
    const FVector Start = GetActorLocation() + FVector(0, 0, 50.f);
    const FVector End = Start + GetActorForwardVector() * TraceDistance;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ArrestTrace), false, this);
    Params.AddIgnoredActor(this);

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

/* ====== ��ٿ� ====== */

void APoliceCH4Character::StartArrestCooldown_Local()
{
    bArrestOnCooldown_Local = true;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ArrestCooldownTimerHandle_Local,
            [this]() { bArrestOnCooldown_Local = false; },
            ArrestCooldown, false
        );
    }
}

void APoliceCH4Character::StartArrestCooldown_Server()
{
    bArrestOnCooldown_Server = true;
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ArrestCooldownTimerHandle_Server,
            [this]() { bArrestOnCooldown_Server = false; },
            ArrestCooldown, false
        );
    }
}
