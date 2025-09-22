#include "Character/CH4Character.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "AnimInstance/CH4AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameMode/CH4GameMode.h"
#include "PlayerState/CH4PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Item/BaseItem.h"
#include "Item/CokeItem.h"

ACH4Character::ACH4Character()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 변수 초기화
	DefaultMappingContext = nullptr;
	JumpAction = nullptr;
	LookAction = nullptr;
	MoveAction = nullptr;
	UseItemAction = nullptr;
	Speed = 0.f;
	bIsJumping = false;
	bIsRunning = false;
	bUsingItem = false;
	HeldItem = nullptr; // HeldItem 초기화
	bUsingItem = false; // bUsingItem 초기화

	// 카메라 컴포넌트 생성 및 연결
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	// 캐릭터 이동 설정
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bUseControllerRotationYaw = false;                       // 컨트롤러 회전에 따라 회전하지 않음
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 회전
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f); // 회전 속도

	// 충돌 이벤트 바인딩
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACH4Character::OnOverlapBegin);
}

void ACH4Character::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ACH4Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 클라이언트에서만 이동 변수 업데이트
	if (IsLocallyControlled())
	{
		Speed = GetVelocity().Size();
		bIsJumping = GetCharacterMovement()->IsFalling();
	}

	// 애니메이션 인스턴스에 복제된 변수 및 로컬 변수 전달
	if (UCH4AnimInstance* AnimInst = Cast<UCH4AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		AnimInst->bIsJumping = bIsJumping;
		AnimInst->bIsRunning = bIsRunning;
		AnimInst->Speed = Speed;
		AnimInst->bUsingItem = bUsingItem;
	}
}

void ACH4Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACH4Character::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACH4Character::Look);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(UseItemAction, ETriggerEvent::Started, this, &ACH4Character::UseItemInput);
	}
}

void ACH4Character::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

void ACH4Character::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// 아이템 사용
void ACH4Character::UseItemInput()
{
	UE_LOG(LogTemp, Log, TEXT("HeldItem: %s"), HeldItem ? *HeldItem->GetName() : TEXT("nullptr"));
	// 아이템이 있고, 현재 사용 중이 아니면 사용 시도
	if (HeldItem && !bUsingItem)
	{
		if (HasAuthority())
		{
			UE_LOG(LogTemp, Log, TEXT("호스트 HandleUseItem 호출"));
			HandleUseItem(HeldItem);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("클라이언트 ServerUseItem() RPC 호출"));
			ServerUseItem();
		}
	}
}

// 서버에서 아이템 사용 요청을 받아서 처리
void ACH4Character::ServerUseItem_Implementation()
{
	//UE_LOG(LogTemp, Log, TEXT("ServerUseItem_Implementation() - 서버에서 요청 수신"));
	if (HeldItem && !bUsingItem)
	{
		HandleUseItem(HeldItem);
	}
}

// 실제 아이템 사용 처리
void ACH4Character::HandleUseItem(UBaseItem* Item)
{
	if (!Item)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HandleUseItem() - 아이템 사용 시작: %s"), *Item->GetName());

	bUsingItem = true;
	Item->UseItem(this);
	OnItemUsed(Item);

	if (Item->IsA(UCokeItem::StaticClass()))
	{
		ClientShowSpeedBoostUI();
	}

	HeldItem = nullptr;
	UE_LOG(LogTemp, Log, TEXT("HandleUseItem() - 아이템 사용 완료. HeldItem 초기화"));
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ACH4Character::ResetUsingItem);
}

void ACH4Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACH4Character, bIsJumping);
	DOREPLIFETIME(ACH4Character, bIsRunning);
	DOREPLIFETIME(ACH4Character, bUsingItem);
	DOREPLIFETIME(ACH4Character, HeldItem);
}

// 충돌 이벤트
void ACH4Character::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (IsLocallyControlled())
	{
		if (!OtherActor || HeldItem)
		{
			return;
		}

		if (OtherActor->ActorHasTag(TEXT("Item")))
		{
			UE_LOG(LogTemp, Log, TEXT("Get Item: %s"), *OtherActor->GetName());
			ServerPickupItem(OtherActor);
		}
	}
}

// 서버에서 아이템 줍기 처리
void ACH4Character::ServerPickupItem_Implementation(AActor* ItemActor)
{
	if (!ItemActor || !ItemActor->IsValidLowLevel() || !ItemActor->ActorHasTag(TEXT("Item")))
	{
		return;
	}

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
		ItemActor->Destroy();
	}
}

// 실제 아이템 줍기
void ACH4Character::PickupItem(UBaseItem* Item)
{
	if (!Item || HeldItem)
	{
		return;
	}
	HeldItem = Item;
}

// HeldItem 값 변경 시 UI 갱신
void ACH4Character::OnRep_HeldItem()
{
	UpdateHeldItemUI(HeldItem);
}

// bUsingItem 초기화
void ACH4Character::ResetUsingItem()
{
	bUsingItem = false;
}

// 클라이언트 UI 처리
void ACH4Character::ClientShowSpeedBoostUI_Implementation()
{
	
}
