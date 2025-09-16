#include "Character/ThiefCharacter.h"
#include "Character/CH4Character.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"

AThiefCharacter::AThiefCharacter()
    : Super() // 부모 생성자 호출
{
    bReplicates = true;
    HeldItem = nullptr;
    bUsingItem = false;
}

void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThiefCharacter, HeldItem);
}

void AThiefCharacter::PickupItem(AActor* ItemActor)
{
    if (ItemActor)
    {
        HeldItem = ItemActor;
    }
}

void AThiefCharacter::UseItemInput()
{
    if (HeldItem)
    {
        if (HasAuthority())
        {
            HandleUseItem(HeldItem);
        }
        else
        {
            ServerUseItem();
        }
    }
}

void AThiefCharacter::ServerUseItem_Implementation()
{
    if (HeldItem)
    {
        HandleUseItem(HeldItem);
    }
}

void AThiefCharacter::HandleUseItem(AActor* ItemActor)
{
    if (!ItemActor) return;

    bUsingItem = true;

    if (ItemActor->ActorHasTag("SpeedBoost"))
    {
        // 원래 속도 저장
        float OriginalSpeed = GetCharacterMovement()->MaxWalkSpeed;

        // 속도 증가
        GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed + 300.f;

        // 일정 시간 후 원래 속도로 되돌리기
        FTimerHandle TimerHandle_ResetSpeed;
        GetWorld()->GetTimerManager().SetTimer(
            TimerHandle_ResetSpeed,
            FTimerDelegate::CreateLambda([this, OriginalSpeed]()
                {
                    GetCharacterMovement()->MaxWalkSpeed = OriginalSpeed;
                }),
            5.0f, // 지속 시간 (임시 5초)
            false
        );
        // 클라 전체 이펙트
        MulticastUseSpeedBoost();

        // 소유자에게만 HUD 보이게 하는 클라이언트 RPC 호출 (서버 호출)
        ClientShowSpeedBoostUI();
    }
    else if (ItemActor->ActorHasTag("Trap"))
    {
        if (TrapClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = this;
            SpawnParams.Instigator = GetInstigator();
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // 서버에서 트랩 스폰
            GetWorld()->SpawnActor<AActor>(TrapClass, GetActorLocation(), GetActorRotation(), SpawnParams);

            // 클라 전체에 효과
            MulticastUseTrap();
        }
    }
    else if (ItemActor->ActorHasTag("Clock"))
    {
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            // 제한 시간 5초 줄이기 (음수 방지)
            GS->MatchTime = FMath::Max(0.f, GS->MatchTime - 5.f);

            GS->OnRep_MatchTime(); // 클라 HUD 갱신 트리거
        }

        // 클라 전체 이펙트
        MulticastUseClock();
    }

    // 아이템 제거
    ItemActor->Destroy();
    HeldItem = nullptr;

    //아이템 효과 해제(안넣으면 영원히 사용됨) 이팩트 재생을 위해 딜레이를 넣었는데 필요 없으면 bUsingItem = false; 만 남기면 됨 
    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            bUsingItem = false;
        });
}

void AThiefCharacter::MulticastUseSpeedBoost_Implementation()
{
    // 모든 클라에서 보여줄 이펙트/사운드
    OnSpeedBoostEffect();
    UE_LOG(LogTemp, Log, TEXT("Speed Boost Effect"));
}

void AThiefCharacter::MulticastUseTrap_Implementation()
{
    // 덫 설치 효과
    OnTrapEffect();
    UE_LOG(LogTemp, Log, TEXT("Trap Placed"));
}

void AThiefCharacter::MulticastUseClock_Implementation()
{
    // 시계 효과
    OnClockEffect();
    UE_LOG(LogTemp, Log, TEXT("Clock Used Time Reduced."));
}

// 경찰에게 잡혔을 때
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    // GameMode 가져오기
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        // 도둑 제거 처리
        Destroy(); // 혹은 상태 전환

        // GameState 가져오기
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            GS->OnRep_RemainingThieves(); // 서버에서 바로 UI 갱신 트리거
        }

        // 클라 전용 실패 UI 띄우기
        ClientOnTrapped();
    }
}

// Trap에 걸렸을 때 UI 표시
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnTrapEffect();
}

// 블루프린트에서 구현할 수 있도록 선언
UFUNCTION(BlueprintImplementableEvent, Category = "UI")
void OnTrappedUI();

// 이속 아이템 사용 UI 표시
void AThiefCharacter::ClientShowSpeedBoostUI_Implementation()
{
    ShowSpeedBoostUI();
}

// 블루프린트에서 구현할 수 있도록 선언
UFUNCTION(BlueprintImplementableEvent, Category = "UI")
void ShowSpeedBoostUI();
