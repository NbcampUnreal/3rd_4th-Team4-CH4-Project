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
#include "Kismet/GameplayStatics.h"
#include "PlayerController/CH4PlayerController.h"
#include "IngameUI/CH4UserWidget.h"
#include "Item/BaseItem.h"
#include "Item/PickUp.h"
#include "Item/ClockItem.h"

ACH4Character::ACH4Character()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// 변수 초기화
	DefaultMappingContext = nullptr;
	JumpAction = nullptr;
	LookAction = nullptr;
	MoveAction = nullptr;
	UseSlot1Action = nullptr;
	UseSlot2Action = nullptr;
	Speed = 0.f;
	bIsJumping = false;
	bIsRunning = false;
	bUsingItem = false;
	bUsingItem = false;

	// 인벤토리 크기 2로 설정 (슬롯 2칸)
	Inventory.SetNum(2);

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
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	// 충돌 이벤트 바인딩
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ACH4Character::OnOverlapBegin);
}

void ACH4Character::BeginPlay()
{
	Super::BeginPlay();

	if (ACH4PlayerController* PlayerController = Cast<ACH4PlayerController>(Controller))
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
		AnimInst->bIsStunned = bIsStunned;
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

		EnhancedInputComponent->BindAction(UseSlot1Action, ETriggerEvent::Started, this, &ACH4Character::UseSlot1);
		EnhancedInputComponent->BindAction(UseSlot2Action, ETriggerEvent::Started, this, &ACH4Character::UseSlot2);
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

void ACH4Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ACH4Character, bIsJumping, COND_SkipOwner);
	DOREPLIFETIME(ACH4Character, bIsRunning);
	//DOREPLIFETIME(ACH4Character, Inventory);
	DOREPLIFETIME(ACH4Character, CurrentMaxWalkSpeed);
	DOREPLIFETIME(ACH4Character, Speed);
	DOREPLIFETIME(ACH4Character, bIsDead);
	DOREPLIFETIME(ACH4Character, bIsStunned);
}

//void ACH4Character::OnRep_Inventory()
//{
//	// Inventory 배열에 변경 사항이 생겼을 때 클라이언트에서 호출
//	if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(GetController()))
//	{
//		if (MyPC->MyHUDWidget)
//		{
//			MyPC->MyHUDWidget->UpdateInventoryUI(Inventory);
//			UE_LOG(LogTemp, Warning, TEXT("Client: UI Update SUCCESS."));
//		}
//	}
//	UE_LOG(LogTemp, Warning, TEXT("Client: Inventory Array RepNotify Triggered. Updating UI."));
//}

void ACH4Character::SetCharacterMaxWalkSpeed(float NewMaxWalkSpeed)
{
	// 서버에서만 실행
	if (!HasAuthority()) return;

	// 복제 변수를 변경 (클라이언트에게 전달됨)
	CurrentMaxWalkSpeed = NewMaxWalkSpeed;

	OnRep_MaxWalkSpeed();
}

void ACH4Character::OnRep_MaxWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentMaxWalkSpeed;
	UE_LOG(LogTemp, Warning, TEXT("Client/Server: MaxWalkSpeed updated to %f"), CurrentMaxWalkSpeed);

	if (!HasAuthority()) // 클라이언트에서만 실행
	{
		// 클라이언트의 MaxWalkSpeed가 기본 걷기 속도보다 높으면 달리기
		bIsRunning = (GetCharacterMovement()->MaxWalkSpeed > WalkSpeed + 10.0f);
	}

	bIsRunning = (GetCharacterMovement()->MaxWalkSpeed > WalkSpeed + 10.0f);
}

void ACH4Character::ClientAddItem_Implementation(FName ItemName)
{
	// 서버에서 아이템을 먹었다
	// 클라이언트에서 인벤토리를 복제받는게 아니라 RPC를 통해 클라이언트가 아이템을 새로 생성한다
	CreateItem(ItemName);

	if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(GetController()))
	{
		if (MyPC->MyHUDWidget)
		{
			MyPC->MyHUDWidget->UpdateInventoryUI(Inventory);
			UE_LOG(LogTemp, Warning, TEXT("Client: UI Update SUCCESS."));
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Client: Inventory Array RepNotify Triggered. Updating UI."));
}

void ACH4Character::ServerHandleDeath_Implementation()
{
}

void ACH4Character::MulticastPlayDeathAnimation_Implementation()
{

	if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
	{
		if (DieMontage)
		{
			float MontageDuration = AnimInst->Montage_Play(DieMontage);

			GetWorldTimerManager().SetTimer(
				DestroyTimerHandle,
				this,
				&ACH4Character::RemoveCharacterAfterDeath,
				MontageDuration, // 애니메이션 재생 시간
				false
			);
		}
		else
		{
			RemoveCharacterAfterDeath(); // 몽타주가 없으면 바로 삭제
		}
	}
	else
	{
		RemoveCharacterAfterDeath();
	}
}

// 충돌 이벤트
void ACH4Character::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// 서버에서만 로직 실행
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor)
	{
		return;
	}

	if (OtherActor->ActorHasTag("Trap"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit a trap Character stunned."));
		ServerPlayStunAnimation(); // 서버에서 기절 애니메이션 재생
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Character detected conflict with %s"), *OtherActor->GetName());

	// 충돌한 액터를 APickUp 클래스로 캐스팅
	if (APickUp* PickUpActor = Cast<APickUp>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("crashed actor PickUp item. Item class: %s"), *PickUpActor->GetClass()->GetName());

		// 아이템 클래스가 유효한지 확인
		if (PickUpActor->ItemClass)
		{
			// 서버에서 아이템 객체를 생성
			UBaseItem* NewItem = NewObject<UBaseItem>(this, PickUpActor->ItemClass);
			if (NewItem)
			{
				if (UClockItem* Clock = Cast<UClockItem>(NewItem))
				{
					// PickUp 먼저 제거
					PickUpActor->Destroy();

					// 즉시 사용
					Clock->UseItem(this);

					// UObject 제거
					NewItem->ConditionalBeginDestroy();

					return;
				}

				// 인벤토리에 아이템 추가
				for (int32 i = 0; i < Inventory.Num(); i++)
				{
					if (Inventory[i] == nullptr)
					{
						Inventory[i] = NewItem;
						UE_LOG(LogTemp, Warning, TEXT("Added item '%s' to inventory slot %d"), *NewItem->GetClass()->GetName(), i);

						// ClientUpdateInventoryUI(); // 클라이언트 RPC 호출
						ClientAddItem(NewItem->GetClass()->GetFName());

						PickUpActor->Destroy();
						UE_LOG(LogTemp, Warning, TEXT("Destroyed the PickUp actor in the world"));

						return; // 아이템 획득 및 처리 완료
					}
				}

				// 인벤토리가 가득 찼으면 아이템 삭제
				NewItem->ConditionalBeginDestroy();
				UE_LOG(LogTemp, Warning, TEXT("Inventory is full"));
			}
		}
	}
}

// 인벤토리 UI 업데이트 클라이언트 RPC
void ACH4Character::ClientUpdateInventoryUI_Implementation()
{
	if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(GetController()))
	{
		if (MyPC->MyHUDWidget)
		{
			MyPC->MyHUDWidget->UpdateInventoryUI(Inventory);
			UE_LOG(LogTemp, Warning, TEXT("Client: UI Update SUCCESS."));
		}
	}
}

// 아이템 사용
void ACH4Character::UseSlot1()
{
	// 로컬에서만 서버 RPC를 호출
	if (IsLocallyControlled())
	{
		ServerUseItem(0);
		Inventory[0] = nullptr;
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(GetController()))
		{
			if (MyPC->MyHUDWidget)
			{
				MyPC->MyHUDWidget->UpdateInventoryUI(Inventory);
			}
		}
	}
}

void ACH4Character::UseSlot2()
{
	// 로컬에서만 서버 RPC를 호출
	if (IsLocallyControlled())
	{
		ServerUseItem(1);
		Inventory[1] = nullptr;
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(GetController()))
		{
			if (MyPC->MyHUDWidget)
			{
				MyPC->MyHUDWidget->UpdateInventoryUI(Inventory);
			}
		}
	}
}

// 서버에서 아이템 사용 요청을 받아서 처리 (서버)
void ACH4Character::ServerUseItem_Implementation(int32 SlotIndex)
{
	// 인벤토리, 아이템이 존재하고 현재 아이템을 사용 중이 아닐 때만 진행
	if (!Inventory.IsValidIndex(SlotIndex) || !Inventory[SlotIndex] || bUsingItem)
	{
		UE_LOG(LogTemp, Error, TEXT("failed to use. Index: %d, Item Validity: %d, In Use: %d"),
			SlotIndex, Inventory.IsValidIndex(SlotIndex) && Inventory[SlotIndex], bUsingItem);
		return;
	}

	// 아이템 사용 상태 변경
	bUsingItem = true;

	Inventory[SlotIndex]->UseItem(this);
	UE_LOG(LogTemp, Warning, TEXT("Server: Use items in slot %d"), SlotIndex);

	// 아이템 사용 후 인벤토리에서 제거
	Inventory[SlotIndex] = nullptr;

	// UI 업데이트를 위해 클라이언트로 RPC 호출
	ClientUpdateInventoryUI();

	// 일정 시간 후에 다시 아이템을 사용할 수 있도록 설정
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ACH4Character::ResetUsingItem);
}

void ACH4Character::ServerResetMovementSpeed_Implementation()
{
	// 기본 걷기 속도로 되돌림
	CurrentMaxWalkSpeed = WalkSpeed;
	OnRep_MaxWalkSpeed(); // 클라이언트에도 변경된 속도 동기화

	UE_LOG(LogTemp, Warning, TEXT("Server: Speed reset to WalkSpeed"));
}

// 아이템 사용 상태를 초기화
void ACH4Character::ResetUsingItem()
{
	bUsingItem = false;
	UE_LOG(LogTemp, Warning, TEXT("reset use item"));
}

void ACH4Character::Die()
{
	// 이미 죽은 상태라면 함수를 종료
	if (bIsDead) return;

	// 서버에서만 bIsDead 상태를 변경
	if (HasAuthority())
	{
		bIsDead = true;
		OnRep_IsDead();
	}
}

void ACH4Character::OnRep_IsDead()
{
	if (bIsDead)
	{
		// 입력 막기
		DisableInput(Cast<ACH4PlayerController>(GetController()));
	}
}

void ACH4Character::RemoveCharacterAfterDeath()
{
	if (HasAuthority())
	{
		Destroy();
		UE_LOG(LogTemp, Warning, TEXT("%s: Character destroyed after death"), *GetName());
	}
}

void ACH4Character::ServerPlayStunAnimation_Implementation()
{
	if (bIsStunned) return;

	bIsStunned = true;
	PlayStunAnimation(); // 서버 자신에게 애니메이션 재생 시작

	// 애니메이션이 끝나면 상태를 원래대로 돌릴 타이머를 설정
	const float StunAnimDuration = 2.8f;
	GetWorldTimerManager().SetTimer(
		RunSpeedTimerHandle,
		this,
		&ACH4Character::ResetStunState,
		StunAnimDuration,
		false
	);
}

void ACH4Character::PlayStunAnimation()
{
	if (UCH4AnimInstance* AnimInst = Cast<UCH4AnimInstance>(GetMesh()->GetAnimInstance()))
	{
		if (StunMontage)
		{
			AnimInst->Montage_Play(StunMontage);
			UE_LOG(LogTemp, Warning, TEXT("%s: Play Stun Montage"), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("StunMontage is NULL on %s"), *GetName());
		}
	}
}

void ACH4Character::ResetStunState()
{
	if (!HasAuthority()) return;

	bIsStunned = false; // 복제
	UE_LOG(LogTemp, Warning, TEXT("Server: Reset Stun State"));
}

