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

ACH4Character::ACH4Character()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// ���� �ʱ�ȭ
	DefaultMappingContext = nullptr;
	JumpAction = nullptr;
	LookAction = nullptr;
	MoveAction = nullptr;
	SprintAction = nullptr;
	UseItemAction = nullptr;
	Speed = 0.f;
	bIsJumping = false;
	bIsRunning = false;
	bUsingItem = false;

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
	bUseControllerRotationYaw = false;                       // ��Ʈ�ѷ� ȸ���� ���� ȸ������ ����
	GetCharacterMovement()->bOrientRotationToMovement = true; // �̵� �������� ȸ��
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f); // ȸ�� �ӵ�
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

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACH4Character::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACH4Character::Sprint);

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

void ACH4Character::Sprint(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		// �޸��� Ű�� ������ ��
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		bIsRunning = true;
	}
	else
	{
		// �޸��� Ű���� ���� ���� ��
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		bIsRunning = false;
	}
}
void ACH4Character::UseItemInput()
{
}

void ACH4Character::ServerUseItem_Implementation()
{
}

void ACH4Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACH4Character, bIsJumping);
	DOREPLIFETIME(ACH4Character, bIsRunning);
	DOREPLIFETIME(ACH4Character, bUsingItem);
}

void ACH4Character::HandleUseItem(UBaseItem* Item)
{
	if (!Item)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("HandleUseItem �Լ� ȣ��."));
}