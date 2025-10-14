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

/* 잠금 중 매 틱: XY 속도/입력 제거 + XY를 앵커로 고정 (Z는 그대로 → 자연 하강) */
void APoliceCH4Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bMovementLocked)
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            // 속도에서 XY 제거 (수직 Z는 보존)
            FVector V = Move->Velocity;
            V.X = 0.f;
            V.Y = 0.f;
            Move->Velocity = V;

            // 남아있는 입력 제거
            ConsumeMovementInputVector();

            // XY 위치를 잠금 시작 시점으로 고정 (Z는 현재값 유지)
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
    SetMovementLocked(true);          // 로컬 즉시 잠금

    ServerTryArrest();
    StartArrestCooldown_Local();
}

void APoliceCH4Character::ServerTryArrest_Implementation()
{
    APlayerController* MyPC = Cast<APlayerController>(GetController());
    if (!MyPC) return;
    if (bArrestOnCooldown_Server) return;
    StartArrestCooldown_Server();

    // 서버도 잠금 (권한 측 일관성)
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

/* ====== 몽타주 재생 ====== */

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

                // 종료 델리게이트: 중복 제거 후 바인딩
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
    // TODO: 위젯 연동
}

/* ============== 이동 잠금/해제 (중력 적용, 루트모션 차단, XY 앵커) ============== */

void APoliceCH4Character::SetMovementLocked(bool bLocked)
{
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        if (bLocked)
        {
            if (bMovementLocked) return;
            bMovementLocked = true;

            // 복구용 저장
            SavedMovementMode = Move->MovementMode;
            SavedAirControl = Move->AirControl;
            SavedOrientToMove = Move->bOrientRotationToMovement;
            SavedUseCtrlYaw = bUseControllerRotationYaw;

            // 루트모션 추출 차단
            if (UAnimInstance* Anim = (GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
            {
                Anim->SetRootMotionMode(ERootMotionMode::NoRootMotionExtraction);
            }

            // XY 속도만 제거(수직 Z 보존 → ‘툭’ 떨어지지 않음)
            FVector V = Move->Velocity;
            V.X = 0.f;
            V.Y = 0.f;
            Move->Velocity = V;

            // 회전/제어 제한
            Move->AirControl = 0.f;
            Move->bOrientRotationToMovement = false;
            bUseControllerRotationYaw = false;

            // 중력 적용(공중이면 계속 탄도, 지면이면 그대로 유지)
            Move->SetMovementMode(MOVE_Falling);
            bPressedJump = false;

            // XY 앵커 저장
            const FVector L = GetActorLocation();
            LockAnchorXY = FVector2D(L.X, L.Y);

            // 남아있는 입력 제거
            ConsumeMovementInputVector();
        }
        else
        {
            if (!bMovementLocked) return;
            bMovementLocked = false;

            // 복구
            Move->AirControl = SavedAirControl;
            Move->bOrientRotationToMovement = SavedOrientToMove;
            bUseControllerRotationYaw = SavedUseCtrlYaw;

            Move->SetMovementMode(MOVE_Walking);

            // 루트모션 모드 기본값 복귀
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

/* ====== 쿨다운 ====== */

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
