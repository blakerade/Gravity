// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawnPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VectorTypes.h"
#include "Chaos/SpatialAccelerationCollection.h"
#include "Gravity/Flooring/FloorBase.h"
#include "Gravity/Sphere/GravitySphere.h"

ABasePawnPlayer::ABasePawnPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	Capsule = CreateDefaultSubobject<UCapsuleComponent>("Capsule");
	SetRootComponent(Capsule);
	Capsule->SetCapsuleHalfHeight(90.f);
	Capsule->SetCapsuleRadius(30.f);
	Skeleton = CreateDefaultSubobject<USkeletalMeshComponent>("Skeleton");
	Skeleton->SetupAttachment(RootComponent);
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(Skeleton);
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	
}

void ABasePawnPlayer::BeginPlay()
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
			Subsystem->AddMappingContext(CharacterMovementMapping, 1.f);
		}
	}
	if(Capsule)
	{
		Capsule->OnComponentHit.AddDynamic(this, &ABasePawnPlayer::OnFloorHit);
	}
}

void ABasePawnPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PreformGravity(DeltaTime);
	PreformPlayerMovement();
	FindSphere();
}

void ABasePawnPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		if (MoveAction && AirMoveAction && LookAction && JumpAction && CrouchAction && MagnetizeAction && BoostAction)
		{
			PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Move);
			PlayerEnhancedInputComponent->BindAction(AirMoveAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::AirMove);
			PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Look);
			PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Jump);
			PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Crouch);
			PlayerEnhancedInputComponent->BindAction(MagnetizeAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Magnetize);
			PlayerEnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Boost);
		}
	}
}

void ABasePawnPlayer::Move(const FInputActionValue& ActionValue)
{
	//Adds to ControlInputValue, which is used in PreformPlayerMovement
	const FVector2D Value = ActionValue.Get<FVector2D>();
	//TODO Add differences for forward/backwards/lateral movement
	AddMovementInput(GetActorForwardVector() * Value.Y);
	AddMovementInput(GetActorRightVector() * Value.X);
}

void ABasePawnPlayer::AirMove(const FInputActionValue& ActionValue)
{
	//Adds to ControlInputValue, which is used in PreformPlayerMovement
	if(!bContactedWithFloor)
	{
		const float Value = ActionValue.Get<float>();
		AddMovementInput(GetActorUpVector() * Value);
	}
}

void ABasePawnPlayer::Look(const FInputActionValue& ActionValue)
{
	const FVector2D Value = ActionValue.Get<FVector2D>();
	//Have to Rotate the actor without controller because controller will only rotate in World coordinates
	AddActorLocalRotation(FRotator(0.f,Value.X,0.f));
	//Have to Rotate the SpringArm without controller because the controller will only rotate in World coordinates
	SpringArm->AddLocalRotation(FRotator(Value.Y,0.f,0.f));
}

void ABasePawnPlayer::Jump(const FInputActionValue& ActionValue)
{
	if(bContactedWithFloor && bIsMagnetized)
	{
		bContactedWithFloor = false;
		Capsule->AddImpulse(GetActorUpVector() * JumpVelocity);
		//Commented out because we no longer add a constraint on landing, could change in the future
		// Capsule->SetConstraintMode(EDOFMode::None);
		Capsule->SetLinearDamping(AirFriction);
	}
	if(bContactedWithSphere && bIsMagnetized)
	{
		bContactedWithSphere = false;
		Capsule->AddImpulse(GetActorUpVector() * JumpVelocity);
		//Commented out because we no longer add a constraint on landing, could change in the future
		// Capsule->SetConstraintMode(EDOFMode::None);
		Capsule->SetLinearDamping(AirFriction);
	}
}

void ABasePawnPlayer::Crouch(const FInputActionValue& ActionValue)
{
}

void ABasePawnPlayer::Magnetize(const FInputActionValue& ActionValue)
{
	if(bContactedWithSphere || bContactedWithFloor)
	{
		Jump(1.f);
	}
	bIsMagnetized = !bIsMagnetized;

}

void ABasePawnPlayer::Boost(const FInputActionValue& ActionValue)
{
	bCanBoost = BoostCount >= 2 ? false : true;
	if(bCanBoost)
	{
		Capsule->SetPhysicsLinearVelocity(GetVelocity() / BoostCurrentVelocityReduction);
		if(ControlInputVector.Size() != 0.f)
		{
			Capsule->AddImpulse(ControlInputVector * BoostSpeed);
			BoostCount++;
			GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &ABasePawnPlayer::BoostCountConsumer, BoostRechargeRate);
		}
		else
		{
			Capsule->AddImpulse(GetVelocity().GetSafeNormal() * BoostSpeed);
			BoostCount++;
			GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &ABasePawnPlayer::BoostCountConsumer, BoostRechargeRate);
		}
	}
}

void ABasePawnPlayer::BoostCountConsumer()
{
	BoostCount--;
}

void ABasePawnPlayer::PreformPlayerMovement()
{
	if(!bContactedWithFloor && !bContactedWithSphere)
	{
		Capsule->AddImpulse(ControlInputVector * AirSpeed);
		ConsumeMovementInputVector();
	}
	else if((bContactedWithFloor || bContactedWithSphere) && bIsMagnetized)
	{
		Capsule->AddImpulse(ControlInputVector * GroundSpeed);
        ConsumeMovementInputVector();
	}

}

void ABasePawnPlayer::PreformGravity(float DeltaTime)
{
	if(CurrentGravity.Size() > 0.f && !bContactedWithFloor && bIsMagnetized)
	{
		Capsule->AddForce(CurrentGravity);
		OrientToGravity(CurrentGravity, DeltaTime);
	}
	else if(!bContactedWithFloor && bIsMagnetized && bIsInsideSphere && !bContactedWithSphere)
	{
		//Push us away from the center of the sphere
		const FVector AwayFromCenter = GetActorLocation() -SphereCenter;
		Capsule->AddForce(AwayFromCenter.GetSafeNormal() * SphereGravityStrength);
		OrientToGravity(AwayFromCenter, DeltaTime);
	}
}

void ABasePawnPlayer::FindSphere()
{
	//Trying to set these variables on BeginPlay() with GravitySphere doesn't work, this is currently the best way I can think to set these variables
	if(bIsInsideSphere && !Sphere)
	{
		TArray<FHitResult> HitResults;
		const FCollisionQueryParams TraceParams;
		GetWorld()->LineTraceMultiByChannel(HitResults, GetActorLocation(), GetActorLocation() + (FVector::UpVector * 200000.f), ECollisionChannel::ECC_Visibility, TraceParams);
		for(FHitResult VisHitResults: HitResults)
		{
			if(AGravitySphere* LevelSphere = Cast<AGravitySphere>(VisHitResults.GetActor()))
			{
				Sphere = LevelSphere;
				SphereCenter = Sphere->GetActorLocation();
				return;
			}
		}
	}
}

void ABasePawnPlayer::OrientToGravity(FVector GravityToOrientTo, float DeltaTime)
{
	
	//If we are going the same way as gravity, use MakeFromXY to reduce amount of unnecessary pivoting, could probably use even more improvement
	if(FVector::DotProduct(GetVelocity(), GravityToOrientTo) >= 1.f)
	{
		FeetToGravity = FRotationMatrix::MakeFromZY(-GravityToOrientTo, GetActorRightVector());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), FeetToGravity.Rotator(), DeltaTime, 2.f);
		SetActorRotation(InterpRotation);
	}
	else
	{
		//If gravity is any other direction then this MakeFromZX should give us the smoothest rotation
		FeetToGravity = FRotationMatrix::MakeFromZX(-GravityToOrientTo, GetActorForwardVector());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), FeetToGravity.Rotator(), DeltaTime, 2.f);
		SetActorRotation(InterpRotation);
	}
}

void ABasePawnPlayer::OnFloorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(AFloorBase* Floor = Cast<AFloorBase>(OtherActor))
	{
		if(Capsule && !bContactedWithFloor && bIsMagnetized)	
		{
			// Commented out actions that probably do not help when landed but will stay here as possible debugging later on////////////////////////////////
			// Capsule->SetConstraintMode(EDOFMode::CustomPlane);
			// Capsule->BodyInstance.CustomDOFPlaneNormal = GetActorRightVector();
			// Capsule->SetPhysicsLinearVelocity(FVector::ZeroVector);
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			//If the Rinterpto isn't done we still need to be rotated corrected when we land
			SetActorRotation(FeetToGravity.Rotator());
			//Important bool for other functionality
			bContactedWithFloor = true;
			//Stops the capsule from falling over
			Capsule->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			//Set damping to a high value so that when we are walking it doesn't feel like we are skating
			Capsule->SetLinearDamping(FloorFriction);
		}
	}
	if(AGravitySphere* LevelSphere = Cast<AGravitySphere>(OtherActor))
	{
		if(Capsule && bIsMagnetized)
		{
			//If the Rinterpto isn't done we still need to be rotated corrected when we land //THIS NEEDS TO BE FIXED
			SetActorRotation(FRotationMatrix::MakeFromZX(SphereCenter - GetActorLocation(), GetActorForwardVector()).Rotator());
			//Important bool for other functionality
			bContactedWithSphere = true;
			//Stops the capsule from falling over
			Capsule->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			//Set damping to a high value so that when we are walking it doesn't feel like we are skating
			Capsule->SetLinearDamping(FloorFriction);
		}
	}
}

void ABasePawnPlayer::SetContactedWith(bool bIsContactedWithAFloor)
{
	if(bContactedWithFloor)
	{
		bContactedWithFloor = bIsContactedWithAFloor;
		Capsule->SetLinearDamping(AirFriction);
		
	}
	if(bContactedWithSphere)
	{
		bContactedWithSphere = bIsContactedWithAFloor;
		Capsule->SetLinearDamping(AirFriction);
	}
}


