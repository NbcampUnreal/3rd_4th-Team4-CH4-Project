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

// ▼ 필요 시: 아이템 클래스/인터페이스 헤더

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
    // 클라 입력 → 서버가 판정
    ServerTryArrest();
}

void APoliceCH4Character::ServerTryArrest_Implementation()
{
    AActor* Target = FindArrestTarget(ArrestTraceDistance, ArrestTraceRadius);
    bool bSuccess = false;

    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        // 타겟의 PlayerState로 역할 확인 (없으면 시민/AI 취급)
        if (ACH4PlayerState* TargetPS = TargetPawn->GetPlayerState<ACH4PlayerState>())
        {
            if (TargetPS->PlayerRole == EPlayerRole::Thief)
            {
                bSuccess = true;

                // GameMode에 실제 체포 처리 위임(도둑 수 감소, Destroy, 승리 조건 등)
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
                // 경찰은 체포 대상 아님 → 실패 처리(연출/경고 UI)
                bSuccess = false;
            }
            else
            {
                // Unassigned 등 → 실패 취급(시민/AI와 유사)
                bSuccess = false;

                // 시민 오인 체포면 GameMode 측에서 한도 차감/사직 로직 수행 중
                if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
                {
                    if (APlayerController* MyPC = Cast<APlayerController>(GetController()))
                    {
                        GM->HandleArrest(MyPC, TargetPawn); // 내부에서 AI면 OnAICaught 경로
                    }
                }
            }
        }
        else
        {
            // PlayerState가 없으면 AI/시민 취급 → GameMode로 위임
            if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
            {
                if (APlayerController* MyPC = Cast<APlayerController>(GetController()))
                {
                    GM->HandleArrest(MyPC, TargetPawn);
                }
            }
            bSuccess = false; // 시민(무고)로 간주
        }
    }

    // 연출 브로드캐스트
    MulticastPlayArrestFX(bSuccess);

    // 체포한 본인에게만 결과 UI
    ClientShowArrestResultUI(bSuccess);
}

void APoliceCH4Character::MulticastPlayArrestFX_Implementation(bool bSuccess)
{
    // TODO: 성공/실패에 따른 이펙트/사운드/애님
    UE_LOG(LogTemp, Log, TEXT("[ArrestFX] %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAIL"));
}

void APoliceCH4Character::ClientShowArrestResultUI_Implementation(bool bSuccess)
{
    // TODO: 블루프린트 위젯 연동
    // ex) 성공: “도둑 체포!”, 실패: “무고 체포 경고”
}

/* ================= Trace Helper ================= */

AActor* APoliceCH4Character::FindArrestTarget(float TraceDistance, float Radius) const
{
    const FVector Start = GetActorLocation() + FVector(0, 0, 50.f);
    const FVector End = Start + GetActorForwardVector() * TraceDistance;

    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ArrestTrace), false, this);

    // Pawn 채널에서 Block 되도록 프리셋 구성 필요
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




