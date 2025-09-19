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
    // 충돌한 액터가 없거나, 이미 아이템을 가지고 있으면 무시
    if (!OtherActor || HeldItem)
    {
        return;
    }

    // 충돌한 액터에 "Item" 태그가 있는지 확인
    if (OtherActor->ActorHasTag(TEXT("Item")))
    {
        UE_LOG(LogTemp, Log, TEXT("Get Item: %s"), *OtherActor->GetName());
        // 아이템 줍기 요청을 서버에 보내기
        ServerPickupItem(OtherActor);
    }
}

// 서버에서 아이템 줍기 처리
void AThiefCharacter::ServerPickupItem_Implementation(AActor* ItemActor)
{
    // 네트워크로 전달된 ItemActor가 유효한지 다시 한번 검증
    if (!ItemActor || !ItemActor->IsValidLowLevel() || !ItemActor->ActorHasTag(TEXT("Item")))
    {
        return;
    }

    // ItemActor에서 아이템 데이터를 가져오는 함수를 찾고 호출
    UBaseItem* Item = nullptr;
    UFunction* Func = ItemActor->FindFunction(FName("GetItemData"));
    if (Func)
    {
        struct FItemGetter { UBaseItem* ReturnValue; };
        FItemGetter ItemGetter;
        ItemActor->ProcessEvent(Func, &ItemGetter);
        Item = ItemGetter.ReturnValue;
    }

    // 아이템 데이터가 유효하면 실제로 아이템을 줍고, 월드에서 해당 액터를 파괴
    if (Item)
    {
        PickupItem(Item);
        ItemActor->Destroy();
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
    // HeldItem 값이 변경될 때마다 블루프린트의 UI를 업데이트하도록 호출
    UpdateHeldItemUI(HeldItem);
}

// 실제 아이템 줍기
void AThiefCharacter::PickupItem(UBaseItem* Item)
{
    // 이미 아이템이 있거나, 줍는 아이템이 유효하지 않으면 무시
    if (!Item || HeldItem)
    {
        return;
    }
    HeldItem = Item;
}

// 아이템 사용 입력
void AThiefCharacter::UseItemInput()
{
    // 아이템이 있고, 현재 사용 중인 상태가 아니라면
    if (HeldItem && !bUsingItem)
    {
        // 서버라면 바로 사용 처리, 아니라면 서버에 사용 요청
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

// 서버에서 아이템 사용 요청을 받아서 처리
void AThiefCharacter::ServerUseItem_Implementation()
{
    if (HeldItem && !bUsingItem)
    {
        HandleUseItem(HeldItem);
    }
}

// 실제 아이템 사용 처리
void AThiefCharacter::HandleUseItem(UBaseItem* Item)
{
    if (!Item)
    {
        return;
    }
    // 아이템 사용 상태로 변경
    bUsingItem = true;

    // 아이템의 UseItem 함수를 호출하여 효과를 발생
    Item->UseItem(this);

    // 블루프린트 이벤트 호출
    OnItemUsed(Item);

    // 특정 아이템(콜라)이라면 속도 증가 UI를 클라이언트에 표시하도록 요청
    if (Item->IsA(UCokeItem::StaticClass()))
    {
        ClientShowSpeedBoostUI();
    }

    // 사용이 끝났으므로 아이템 초기화
    HeldItem = nullptr;

    // 다음 틱(프레임)에 bUsingItem 상태를 다시 초기화
    GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AThiefCharacter::ResetUsingItem);
}

// bUsingItem 초기화
void AThiefCharacter::ResetUsingItem()
{
    bUsingItem = false;
}

// 경찰에게 잡혔을 때 서버에서만 실행
void AThiefCharacter::ServerOnCaughtByPolice_Implementation()
{
    // 게임 모드를 가져와서 캐릭터를 파괴
    if (ACH4GameMode* GM = Cast<ACH4GameMode>(UGameplayStatics::GetGameMode(this)))
    {
        Destroy();

        // 게임 상태를 가져와서 남은 도둑 수를 줄임
        if (ACH4GameStateBase* GS = GetWorld()->GetGameState<ACH4GameStateBase>())
        {
            GS->RemainingThieves = FMath::Max(0, GS->RemainingThieves - 1);
            // OnRep 함수를 수동으로 호출하여 남은 도둑 수 변경을 모든 클라이언트에 알려줌
            GS->OnRep_RemainingThieves();
        }

        // 클라이언트에게 잡혔다는 UI를 보여주도록 요청
        ClientOnTrapped();
    }
}

// 클라이언트 UI 처리
void AThiefCharacter::ClientOnTrapped_Implementation()
{
    OnItemUsed(nullptr);
}

void AThiefCharacter::ClientShowSpeedBoostUI_Implementation()
{
    ShowSpeedBoostUI();
}