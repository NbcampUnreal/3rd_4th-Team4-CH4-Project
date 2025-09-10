#include "Character/CH4Character.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "AnimInstance/CH4AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ACH4Character::ACH4Character()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ACH4Character::BeginPlay()
{
	Super::BeginPlay();
}

void ACH4Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		Speed = GetVelocity().Size();
	}

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

	if (!IsLocallyControlled())
		return;

	// 이동
	PlayerInputComponent->BindAxis("MoveForward", this, &ACH4Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACH4Character::MoveRight);

	// 마우스 회전
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	// 점프
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACH4Character::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACH4Character::JumpReleased);

	// 달리기
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ACH4Character::RunPressed);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ACH4Character::RunReleased);
}

void ACH4Character::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACH4Character::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ACH4Character::JumpPressed()
{
	bIsJumping = true;
	Jump();
}

void ACH4Character::JumpReleased()
{
	bIsJumping = false;
	StopJumping();
}

void ACH4Character::RunPressed()
{
	bIsRunning = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ACH4Character::RunReleased()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ACH4Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACH4Character, bIsJumping);
	DOREPLIFETIME(ACH4Character, bIsRunning);
	DOREPLIFETIME(ACH4Character, bUsingItem);
}
