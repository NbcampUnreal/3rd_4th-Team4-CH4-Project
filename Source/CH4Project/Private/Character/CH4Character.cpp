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

ACH4Character::ACH4Character()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// ���� �ʱ�ȭ
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

	// �κ��丮 ũ�� 2�� ���� (���� 2ĭ)
	Inventory.SetNum(2); 

	// ī�޶� ������Ʈ ���� �� ����
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	// ĳ���� �̵� ����
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);

	// �浹 �̺�Ʈ ���ε�
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

	// Ŭ���̾�Ʈ������ �̵� ���� ������Ʈ
	if (IsLocallyControlled())
	{
		Speed = GetVelocity().Size();
		bIsJumping = GetCharacterMovement()->IsFalling();
	}

	if (HasAuthority()) // ���������� ����
	{
		bIsRunning = (GetVelocity().Size() > WalkSpeed + 10.0f);
	}

	// �ִϸ��̼� �ν��Ͻ��� ������ ���� �� ���� ���� ����
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
	DOREPLIFETIME_CONDITION(ACH4Character, bIsRunning, COND_SkipOwner);
	DOREPLIFETIME(ACH4Character, Inventory);
	DOREPLIFETIME(ACH4Character, CurrentMaxWalkSpeed);
	DOREPLIFETIME(ACH4Character, Speed);
	DOREPLIFETIME(ACH4Character, bIsDead);
}

void ACH4Character::OnRep_MaxWalkSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentMaxWalkSpeed;	
}

// �浹 �̺�Ʈ
void ACH4Character::OnOverlapBegin(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// ���������� ���� ����
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Character detected conflict with %s"), *OtherActor->GetName());

	// �浹�� ���͸� APickUp Ŭ������ ĳ����
	if (APickUp* PickUpActor = Cast<APickUp>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("crashed actor PickUp item. Item class: %s"), *PickUpActor->GetClass()->GetName());

		// ������ Ŭ������ ��ȿ���� Ȯ��
		if (PickUpActor->ItemClass)
		{
			// �������� ������ ��ü�� ����
			UBaseItem* NewItem = NewObject<UBaseItem>(this, PickUpActor->ItemClass);
			if (NewItem)
			{
				// �κ��丮�� ������ �߰�
				for (int32 i = 0; i < Inventory.Num(); i++)
				{
					if (Inventory[i] == nullptr)
					{
						Inventory[i] = NewItem;
						UE_LOG(LogTemp, Warning, TEXT("Added item '%s' to inventory slot %d"), *NewItem->GetClass()->GetName(), i);

						// UI ������ ���� Ŭ���̾�Ʈ�� RPC ȣ��
						ClientUpdateInventoryUI();

						// �������� �ֿ����� �ݴ� ������ ���� �ı�
						PickUpActor->Destroy();
						UE_LOG(LogTemp, Warning, TEXT("Destroyed the PickUp actor in the world"));
						return;
					}
				}

				// �κ��丮�� ���� á���� ������ ����
				NewItem->ConditionalBeginDestroy();
				UE_LOG(LogTemp, Warning, TEXT("Inventory is full"));
			}
		}
	}
}

// �κ��丮 UI ������Ʈ Ŭ���̾�Ʈ RPC
void ACH4Character::ClientUpdateInventoryUI_Implementation()
{
	//UpdateHeldItemUI(Inventory.IsValidIndex(0) ? Inventory[0] : nullptr);
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (ACH4PlayerController* MyPC = Cast<ACH4PlayerController>(PC))
		{
			if (MyPC->MyHUDWidget)
			{
				// ���� ���������� �ּ� ó��
				//MyPC->MyHUDWidget->UpdateInventoryUI(Inventory);
			}
		}
	}
}

// ������ ���
void ACH4Character::UseSlot1()
{
	// ���ÿ����� ���� RPC�� ȣ��
	if (IsLocallyControlled())
	{
		ServerUseItem(0);
	}
}

void ACH4Character::UseSlot2()
{
	// ���ÿ����� ���� RPC�� ȣ��
	if (IsLocallyControlled())
	{
		ServerUseItem(1);
	}
}

// �������� ������ ��� ��û�� �޾Ƽ� ó�� (����)
void ACH4Character::ServerUseItem_Implementation(int32 SlotIndex)
{
	// �κ��丮, �������� �����ϰ� ���� �������� ��� ���� �ƴ� ���� ����
	if (!Inventory.IsValidIndex(SlotIndex) || !Inventory[SlotIndex] || bUsingItem)
	{
		UE_LOG(LogTemp, Error, TEXT("failed to use. Index: %d, Item Validity: %d, In Use: %d"),
			SlotIndex, Inventory.IsValidIndex(SlotIndex) && Inventory[SlotIndex], bUsingItem);
		return;
	}

	// ������ ��� ���� ����
	bUsingItem = true;

	// �������� ��� �Լ� ȣ��
	Inventory[SlotIndex]->UseItem(this);
	UE_LOG(LogTemp, Warning, TEXT("Server: Use items in slot %d"), SlotIndex);

	// ������ ��� �� �κ��丮���� ����
	Inventory[SlotIndex] = nullptr;

	// UI ������Ʈ�� ���� Ŭ���̾�Ʈ�� RPC ȣ��
	ClientUpdateInventoryUI();

	// ���� �ð� �Ŀ� �ٽ� �������� ����� �� �ֵ��� ����
	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ACH4Character::ResetUsingItem);
}

void ACH4Character::ServerResetMovementSpeed_Implementation()
{
	// �⺻ �ȱ� �ӵ��� �ǵ���
	CurrentMaxWalkSpeed = WalkSpeed;
	OnRep_MaxWalkSpeed(); // Ŭ���̾�Ʈ���� ����� �ӵ� ����ȭ

	// �ִϸ��̼� ���¸� ������Ʈ�ϱ� ���� bIsRunning ������ false�� ����
	bIsRunning = false;

	UE_LOG(LogTemp, Warning, TEXT("Server: Speed reset to WalkSpeed"));
}

// ������ ��� ���¸� �ʱ�ȭ
void ACH4Character::ResetUsingItem()
{
	bUsingItem = false;
	UE_LOG(LogTemp, Warning, TEXT("reset use item"));
}

void ACH4Character::Die()
{
	// �̹� ���� ���¶�� �Լ��� ����
	if (bIsDead) return;

	// ���������� bIsDead ���¸� ����
	if (HasAuthority())
	{
		bIsDead = true;
	}
}

void ACH4Character::OnRep_IsDead()
{
	if (bIsDead)
	{
		// �Է� ����
		DisableInput(Cast<APlayerController>(GetController()));

		// �ִϸ��̼� ���
		if (UCH4AnimInstance* AnimInst = Cast<UCH4AnimInstance>(GetMesh()->GetAnimInstance()))
		{
			AnimInst->PlayDeadAnimation();
		}
	}
}
