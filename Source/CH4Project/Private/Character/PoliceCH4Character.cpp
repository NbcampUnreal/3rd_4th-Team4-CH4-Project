#include "Character/PoliceCH4Character.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

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
                    GM->HandleArrest(MyPC, TargetPawn);
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
                    GM->HandleArrest(MyPC, TargetPawn);
                }
            }
        }
        else
        {
            // PlayerState가 없으면 AI/시민 취급 → GameMode로 위임
            if (ACH4GameMode* GM = GetWorld()->GetAuthGameMode<ACH4GameMode>())
            {
                GM->HandleArrest(MyPC, TargetPawn);
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


