// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceShipBase.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"

ASpaceShipBase::ASpaceShipBase()
{
	PrimaryActorTick.bCanEverTick = true;
	ShipBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipBody"));
	SetRootComponent(ShipBody);
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(ShipBody);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void ASpaceShipBase::BeginPlay()
{
	Super::BeginPlay();

	// Make sure that we have a valid PlayerController.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
			Subsystem->ClearAllMappings();

			// Add each mapping context, along with their priority values. Higher values outprioritize lower values.
			Subsystem->AddMappingContext(ShipMapping, 1.f);
		}
	}
}

void ASpaceShipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpaceShipBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		if (MoveAction && YawAction)
		{
			PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, & ASpaceShipBase::Move);
			PlayerEnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Triggered, this, & ASpaceShipBase::Yaw);
		}
	}
}

void ASpaceShipBase::Move(const FInputActionValue& ActionValue)
{
	const FVector Value = ActionValue.Get<FVector>();

	if(Value.X > 0)
	{
		ShipBody->AddForce(GetActorForwardVector() * Value.X * ThusterSpeed);
	}
	else
	{
		ShipBody->AddForce(GetActorForwardVector() * Value.X * BrakeSpeed);
	}
	AddActorLocalRotation(FRotator(FMath::Clamp(Value.Y, -1, 1) * PitchSpeed, 0.f, FMath::Clamp(Value.Z, -1, 1) * RollSpeed));

	
}

void ASpaceShipBase::Yaw(const FInputActionValue& ActionValue)
{
	const float Value = ActionValue.Get<float>();
	AddActorLocalRotation(FRotator(0.f, Value * YawSpeed, 0.f));
}
