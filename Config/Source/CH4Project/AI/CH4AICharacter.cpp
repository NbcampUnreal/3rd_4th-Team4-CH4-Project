// Fill out your copyright notice in the Description page of Project Settings.


#include "CH4AICharacter.h"

// Sets default values
ACH4AICharacter::ACH4AICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACH4AICharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACH4AICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACH4AICharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

