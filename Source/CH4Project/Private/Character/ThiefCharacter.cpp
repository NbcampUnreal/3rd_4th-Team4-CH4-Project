#include "Character/ThiefCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Item/BaseItem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameMode/CH4GameMode.h"
#include "GameState/CH4GameStateBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AThiefCharacter::AThiefCharacter()
{
    bReplicates = true;
    HeldItem = nullptr;
    bUsingItem = false;

    // 충돌 이벤트 바인딩
    GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AThiefCharacter::OnOverlapBegin);
}

// 충돌 이벤트
void AThiefCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor) return;

    // 이미 아이템을 들고 있으면 무시
    if (HeldItem) return;

    // 충돌한 것이 아이템 액터라면
    if (OtherActor->ActorHasTag(TEXT("Item")))
    {
        UE_LOG(LogTemp, Log, TEXT("캐릭터가 아이템과 충돌했습니다: %s"), *OtherActor->GetName());
        ServerPickupItem(OtherActor); // 서버에 요청
    }
}

// 서버에서 아이템 줍기 처리
void AThiefCharacter::ServerPickupItem_Implementation(AActor* ItemActor)
{
    if (!ItemActor) return;

    UBaseItem* Item = nullptr;
    UFunction* Func = ItemActor->FindFunction(FName("GetItemData"));
    if (Func)
    {
        struct FItemGetter { UBaseItem* ReturnValue; };
        FItemGetter ItemGetter;
        ItemActor->ProcessEvent(Func, &ItemGetter);
        Item = ItemGetter.ReturnValue;
    }

    if (Item)
    {
        PickupItem(Item);
        ItemActor->Destroy(); // 서버에서 아이템 제거
    }
}

// HeldItem 복제 등록
void AThiefCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AThiefCharacter, HeldItem);
}

// HeldItem 값 변경 시 UI 갱신
void AThiefCharacter::OnRep_HeldItem()
{
    UpdateHeldItemUI(HeldItem);
}

// 실제 아이템 줍기
void AThiefCharacter::PickupItem(UBaseItem* Item)
{
    if (!Item || HeldItem) return;
    HeldItem = Item;
}

// 아이템 사용 입력
void AThiefCharacter::UseItemInput()
{
    if (HeldItem && !bUsingItem)
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
    if (HeldItem && !bUsingItem)
    {
        HandleUseItem(HeldItem);
    }
}

// 실제 아이템 사용 처리 (상위 클래스 시그니처와 동일)
void AThiefCharacter::HandleUseItem(UBaseItem* ItemActor)
{
    if (!ItemActor) return;

    // 안전하게 UBaseItem로 캐스팅
    UBaseItem* Item = Cast<UBaseItem>(ItemActor);
    if (!Item) return;

    bUsingItem = true;

    // 아이템 사용
    Item->UseItem(this);

    // 블루프린트 이벤트 호출
    OnItemUsed(Item);

    // 클라이언트 UI
    if (Item->IsA(UCokeItem::StaticClass()))
        ClientShowSpeedBoostUI();

    // 사용 후 초기화
    HeldItem = nullptr;

    // 다음 틱에 bUsingItem 해제 (람다 대신 멤버 함수)
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AThiefCharacter::ResetUsingItem);
}

// bUsingItem 초기화
void AThiefCharacter::ResetUsingItem()
{
    bUsingItem = false;
}

// 경찰에게 잡혔을 때 서버 처리
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        Destroy();

        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            GS->OnRep_RemainingThieves();
        }

        ClientOnTrapped();
    }
}

// 클라이언트 UI 처리
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed(nullptr);
}

void AThiefCharacter::ClientShowSpeedBoostUI()
{
    ShowSpeedBoostUI();
}
