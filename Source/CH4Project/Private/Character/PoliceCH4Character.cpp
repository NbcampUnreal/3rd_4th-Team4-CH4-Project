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
    if (!IsLocallyControlled()) return;   //소유자만 입력 허용
    if (bArrestOnCooldown_Local) return;  //로컬 쿨다운 게이트

    // 클라 입력 → 서버가 판정
    PlayLocalArrestMontage();   //입력 즉시 로컬에서 모션 재생
    ServerTryArrest();                    // 서버에 체포 요청
    StartArrestCooldown_Local();          //로컬 쿨다운 시작
}

void APoliceCH4Character::ServerTryArrest_Implementation()
{
    //서버 측 유효성/쿨다운 가드
    APlayerController* MyPC = Cast<APlayerController>(GetController());
    if (!MyPC) return;                        // 컨트롤러 없으면 무시
    if (bArrestOnCooldown_Server) return;     // 서버 쿨다운 중이면 무시
    StartArrestCooldown_Server();             // 서버 쿨다운 시작


    AActor* Target = FindArrestTarget(ArrestTraceDistance, ArrestTraceRadius);
    bool bSuccess = false;
    APawn* TargetPawn = Cast<APawn>(Target);
    if (TargetPawn)
    {
        if (ACH4PlayerState* TargetPS = TargetPawn->GetPlayerState<ACH4PlayerState>())
        {
            bSuccess = (TargetPS->PlayerRole == EPlayerRole::Thief); // UI용 성공 여부
        }

        if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
        {
            GM->HandleArrest(MyPC, TargetPawn); // 통합 위임(도둑/AI/경찰 예외 모두 내부 처리)
        }

        MulticastPlayArrestMontage(); // [추가] 전 클라 동기화 재생
    }
    else
    {
        // 타겟이 없어도 휘두르는 모션을 보이고 싶으면 아래 주석 해제
         MulticastPlayArrestMontage();
    }

    // 연출 브로드캐스트
    MulticastPlayArrestFX(bSuccess);

    // 체포한 본인에게만 결과 UI
    ClientShowArrestResultUI(bSuccess);

}

/* ====== 몽타주 재생 ====== */

void APoliceCH4Character::PlayLocalArrestMontage() // [추가]
{
    if (!ArrestMontage) return;

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (UAnimInstance* Anim = MeshComp->GetAnimInstance())
        {
            if (!Anim->Montage_IsPlaying(ArrestMontage))
            {
                Anim->Montage_Play(ArrestMontage, 1.0f);
            }
        }
    }
}

void APoliceCH4Character::MulticastPlayArrestMontage_Implementation()
{
    // 멀티캐스트는 송신자 포함 모든 클라에서 호출 → PlayLocal 내부에서 중복 재생 방지 체크
    PlayLocalArrestMontage();
}

void APoliceCH4Character::MulticastPlayArrestFX_Implementation(bool bSuccess)
{
    //이 함수는 이펙트/사운드/카메라셰이크 등 연출만 담당(몽타주는 분리)
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

    Params.AddIgnoredActor(this);

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

/* ====== 쿨다운 헬퍼 ====== */

// 로컬 쿨다운 시작: 일정 시간 후 bArrestOnCooldown_Local = false
void APoliceCH4Character::StartArrestCooldown_Local()
{
    bArrestOnCooldown_Local = true;  
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ArrestCooldownTimerHandle_Local,         
            [this]() { bArrestOnCooldown_Local = false; }, // 타이머 끝나면 자동 해제
            ArrestCooldown, false                    
        );
    }
}

// 서버 쿨다운 시작: 일정 시간 후 bArrestOnCooldown_Server = false

void APoliceCH4Character::StartArrestCooldown_Server()
{
    bArrestOnCooldown_Server = true; 
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ArrestCooldownTimerHandle_Server,        
            [this]() { bArrestOnCooldown_Server = false; }, // 타이머 끝나면 자동 해제
            ArrestCooldown, false                     
        );
    }
}


