// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawnPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VectorTypes.h"
#include "Chaos/SpatialAccelerationCollection.h"
#include "Gravity/Flooring/FloorBase.h"
#include "Gravity/Flooring/SphereFloorBase.h"
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

	DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + CurrentGravity, FColor::Blue);
	PerformGravity(DeltaTime);
	PerformPlayerMovement();
	FindSphere();
	SphereFloorContactedGravity(DeltaTime);
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
	//Adds to ControlInputValue, which is used in PerformPlayerMovement
	const FVector2D Value = ActionValue.Get<FVector2D>();

	//Add differences for forward/backwards/lateral movement and if we are in the air or not
	if(GetContactedWith())
	{
		AddMovementInput(GetActorRightVector() * Value.Y * LateralSpeed);
		if(Value.X > 0.f)
		{
			AddMovementInput(GetActorForwardVector() * Value.X * ForwardSpeed);
		}
		else if(Value.X < 0.f)
		{
			AddMovementInput(GetActorForwardVector() * Value.X * BackwardsSpeed);
		}
	}
	else
	{
		AddMovementInput(GetActorForwardVector() * Value.X);
		AddMovementInput(GetActorRightVector() * Value.Y);
	}
	
}

void ABasePawnPlayer::AirMove(const FInputActionValue& ActionValue)
{
	//Adds to ControlInputValue, which is used in PreformPlayerMovement
	if(!GetContactedWith())
	{
		const float Value = ActionValue.Get<float>();
		AddMovementInput(GetActorUpVector() * Value);
	}
}

void ABasePawnPlayer::Look(const FInputActionValue& ActionValue)
{
	const FVector2D Value = ActionValue.Get<FVector2D>();
	
	if(GetContactedWith())
	{
		//Have to Rotate the actor without controller because controller will only rotate in World coordinates causing the character to improperly add yaw and pitch
		AddActorLocalRotation(FRotator(0.f,Value.X,0.f));
	}
	else
	{
		Capsule->AddTorqueInDegrees(GetActorUpVector() * AirRotationSpeed * Value.X, NAME_None, true);
	}
	
	//Have to Rotate the SpringArm without controller because the controller will only rotate in World coordinates causing the character to improperly add yaw and pitch
	const float SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
	if(SpringArmPitch <= -75.f)
	{
		//this if is for how far to look down
		if(Value.Y >= 0.f)
		{
			SpringArm->AddLocalRotation(FRotator(Value.Y,0.f,0.f));
		}
		else if(Value.Y <= 0.f && !GetContactedWith() && !bIsMagnetized)
		{
			Capsule->AddTorqueInRadians(GetActorRightVector() * AirForwardRollSpeed);
		}
	}
	else if(SpringArmPitch >= 70.f)
	{
		//this if is for how far to look up
		if(Value.Y <= 0.f)
		{
			SpringArm->AddLocalRotation(FRotator(Value.Y,0.f,0.f));
		}
		else if(Value.Y >= 0.f && !GetContactedWith() && !bIsMagnetized)
		{
			Capsule->AddTorqueInRadians(GetActorRightVector() * -AirForwardRollSpeed);
		}
	}
	else
	{
		SpringArm->AddLocalRotation(FRotator(Value.Y,0.f,0.f));
	}
	
}

void ABasePawnPlayer::Jump(const FInputActionValue& ActionValue)
{
	if(GetContactedWith() && bIsMagnetized)
	{
		SetContactedWith(false);
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
	if(GetContactedWith())
	{
		Jump(1.f);
	}
	
	bIsMagnetized = !bIsMagnetized;
	
	if(!bIsMagnetized)
	{
		ZeroOutCurrentGravity();
		SetContactedWith(false);
	}
	else if(Gravities.Num() != 0 || SphereFloors.Num() != 0)
	{
		if(Capsule)
		{
			Capsule->SetAllPhysicsLinearVelocity(GetVelocity()/2.f);
			Capsule->SetAllPhysicsAngularVelocityInRadians(Capsule->GetPhysicsAngularVelocityInRadians()/2.f);
		}
		
	}

}

void ABasePawnPlayer::Boost(const FInputActionValue& ActionValue)
{
	bCanBoost = BoostCount >= 2 ? false : true;
	if(bCanBoost)
	{
		Capsule->SetPhysicsLinearVelocity(GetVelocity() / BoostCurrentVelocityReduction);
		if(ControlInputVector.Size() != 0.f)
		{
			Capsule->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
			Capsule->AddImpulse(ControlInputVector * BoostSpeed);
			BoostCount++;
			GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &ABasePawnPlayer::BoostCountConsumer, BoostRechargeRate);
		}
		else
		{
			Capsule->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
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

void ABasePawnPlayer::PerformPlayerMovement()
{
	if(!GetContactedWith())
	{
		Capsule->AddImpulse(ControlInputVector * AirSpeed);
		ConsumeMovementInputVector();
	}
	else if(GetContactedWith() && bIsMagnetized)
	{
		Capsule->AddImpulse(ControlInputVector * GroundSpeed);
        ConsumeMovementInputVector();
	}

}

void ABasePawnPlayer::PerformGravity(float DeltaTime)
{
	if(bIsMagnetized && !GetContactedWith())
	{
		float GravityDistance;
		bool bIsAFloorBase = false;
		bool bIsASphereFloor = false;
		
		if(!bHaveAGravity)
		{
			FindClosestGravity(GravityDistance, bIsAFloorBase);
		}
		IsThereACloserSphereFloor(bIsAFloorBase, GravityDistance, bIsASphereFloor);
		if(bHaveAGravity && !bIsASphereFloor)
		{
			//Pull towards the closet floor
			Capsule->AddForce(CurrentGravity);
			OrientToGravity(CurrentGravity, DeltaTime, GravityDistance);
		}
		else if(bHaveAGravity && bIsASphereFloor)
		{
			//Pull towards the center of sphere floor
			Capsule->AddForce(CurrentGravity * SphereFloorGravityStrength);
			OrientToGravity(CurrentGravity, DeltaTime, GravityDistance);
		}
		else if(bIsInsideSphere)
		{
			//Push us away from the center of the sphere
			const FVector AwayFromCenter = GetActorLocation() -SphereCenter;
			Capsule->AddForce(AwayFromCenter.GetSafeNormal() * SphereGravityStrength);
			OrientToGravity(AwayFromCenter, DeltaTime, GravityDistance);
		}
	}
}

void ABasePawnPlayer::FindClosestGravity(float& OutDistanceToGravity, bool& OutIsAFloorBase)
{
	//Have any gravity triggers added themselves to the gravity array
	if(Gravities.Num() > 0)
	{
		FVector ClosestGravity;
		float DistanceToClosestGravity = 0;
		bool bHaveAHit = false;
		const UWorld* World = GetWorld();
		for(FVector GravityToCheck: Gravities)
		{
			if(World)
			{
				//Reach out in each floors gravity to find which one is the closest
				FHitResult HitResult;
				float FeetLocation = Capsule->GetScaledCapsuleHalfHeight();
				FVector FeetVector = GetActorLocation() - (GetActorUpVector() * FeetLocation);
				World->LineTraceSingleByChannel(HitResult, FeetVector, FeetVector +  (GravityToCheck), ECC_GameTraceChannel1);
				DrawDebugLine(World, FeetVector, FeetVector +(GravityToCheck), FColor::Red, false, 3.f);
				DrawDebugPoint(World, FeetVector, 24.f, FColor::Green, false, 3.f);
				if(HitResult.bBlockingHit)
				{
					//If we already have a gravity set as the current closest
					if(bHaveAHit)
					{
						//Check if this new hit is closer than the current closest gravity and set it as the new closest gravity if so
						if(HitResult.Distance < DistanceToClosestGravity)
						{
							ClosestGravity = GravityToCheck;
							//set the distance to this new closest gravity for future checks
							OutDistanceToGravity = HitResult.Distance;
							//Set the actual variable used for pulling towards a floor
							CurrentGravity = ClosestGravity;
							DistanceToClosestGravity = HitResult.Distance;
						}

					}
					else
					{
						//this is the first hit we have so set it as the closest gravity
						DistanceToClosestGravity = HitResult.Distance;
						ClosestGravity = GravityToCheck;
						bHaveAHit = true;
						OutDistanceToGravity = HitResult.Distance;
						CurrentGravity = ClosestGravity;
						OutIsAFloorBase = true;
						bHaveAGravity = true;
					}
				}
			}
		}
	}
	else
	{
		//Gravity array is empty, dont look for any floors
		ZeroOutCurrentGravity();
	}
}

void ABasePawnPlayer::IsThereACloserSphereFloor(bool bHaveAFloorBase,float& InAndOutGravityDistanceCheck, bool& OutSphereFloorOverride)
{
	//Have any gravity triggers added themselves to the sphere array
	if(SphereFloors.Num() > 0)
	{
		FVector ClosestGravity;
		float DistanceToClosestGravity = 0;
		bool bHaveAHit = false;
		const UWorld* World = GetWorld();
		for(ASphereFloorBase* SpheresToCheck: SphereFloors)
		{
			if(World)
			{
				//Reach out in each floors gravity to find which one is the closest
				FHitResult HitResult;
				World->LineTraceSingleByChannel(HitResult, GetActorLocation(), SpheresToCheck->GetActorLocation(), ECC_GameTraceChannel1);
				DrawDebugLine(World, GetActorLocation(), SpheresToCheck->GetActorLocation(), FColor::Red);
				if(HitResult.bBlockingHit)
				{
					if(bHaveAHit)
					{
						if(HitResult.Distance < DistanceToClosestGravity)
						{
							ClosestGravity = SpheresToCheck->GetActorLocation() - GetActorLocation();
							DistanceToClosestGravity = HitResult.Distance;
						}
						if(HitResult.Distance < InAndOutGravityDistanceCheck)
						{
							CurrentGravity = ClosestGravity;
							InAndOutGravityDistanceCheck = HitResult.Distance;
						}
					}
					else
					{
						if(bHaveAFloorBase)
						{
							if(HitResult.Distance < DistanceToClosestGravity)
							{
								ClosestGravity = SpheresToCheck->GetActorLocation() - GetActorLocation();
							}
							if(HitResult.Distance < InAndOutGravityDistanceCheck)
							{
								CurrentGravity = ClosestGravity;
								OutSphereFloorOverride = true;
								InAndOutGravityDistanceCheck = HitResult.Distance;
							}
						}
						else
						{
							DistanceToClosestGravity = HitResult.Distance;
							ClosestGravity = SpheresToCheck->GetActorLocation() - GetActorLocation();
							bHaveAHit = true;
							CurrentGravity = ClosestGravity;
							OutSphereFloorOverride = true;
							InAndOutGravityDistanceCheck = HitResult.Distance;
							bHaveAGravity = true;
						}
					}
				}
			}
		}
	}
}

void ABasePawnPlayer::FindSphere()
{
	//Trying to set these variables on BeginPlay() with GravitySphere doesn't work (BeginOverlap might actually work, should double check), this is currently the best way I can think to set these variables
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

void ABasePawnPlayer::OrientToGravity(FVector GravityToOrientTo, float DeltaTime, float DistanceToGravity)
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
		if(!bContactedWithFloor && Capsule && bIsMagnetized)	
		{
			if(Floor->GetFloorGravity() == CurrentGravity || Floor->GetFloorGravity() == -CurrentGravity)
			{
				if(FVector::DotProduct(-GetActorUpVector(), CurrentGravity.GetSafeNormal()) < 0.7f)
				{
					Magnetize(1.f);
					ZeroOutCurrentGravity();
					Capsule->AddImpulse(NormalImpulse * KnockBackImpulse);
					Capsule->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
					// Capsule->AddTorqueInRadians(FVector(0.f, 0.f, 45.f));
				}
				else
				{
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
		}
	}
	if(AGravitySphere* LevelSphere = Cast<AGravitySphere>(OtherActor))
	{
		if(Capsule && bIsMagnetized)
		{
			//If the Rinterpto isn't done we still need to be rotated corrected when we land //THIS NEEDS TO BE FIXED
			SetActorRotation(FRotationMatrix::MakeFromZX(SphereCenter - GetActorLocation(), GetActorForwardVector()).Rotator());
			//Important bool for other functionality
			bContactedWithLevelSphere = true;
			//Stops the capsule from falling over
			Capsule->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			//Set damping to a high value so that when we are walking it doesn't feel like we are skating
			Capsule->SetLinearDamping(FloorFriction);
		}
	}
	if(ASphereFloorBase* SphereFloor = Cast<ASphereFloorBase>(OtherActor))
	{
		//If the Rinterpto isn't done we still need to be rotated corrected when we land //THIS NEEDS TO BE FIXED
		SetActorRotation(FRotationMatrix::MakeFromZX(GetActorLocation() - SphereFloor->GetActorLocation(), GetActorForwardVector()).Rotator());
		//Important bool for other functionality
		bContactedWithSphereFloor = true;
		//Stops the capsule from falling over
		Capsule->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
		//Set damping to a high value so that when we are walking it doesn't feel like we are skating
		Capsule->SetLinearDamping(FloorFriction);
		//Set what sphere floor with are contacted with so that we can continue to be pulled and oriented to it
		SphereContactedWith = SphereFloor;
	}
}

void ABasePawnPlayer::SphereFloorContactedGravity(float DeltaTime)
{
	if(bContactedWithSphereFloor && SphereContactedWith)
	{
		//Pull towards the center of sphere floor while we are contacted with the floor
		const FVector ToSphereGravity = (SphereContactedWith->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		Capsule->AddForce(ToSphereGravity * SphereFloorContactedForceCorrection);
		OrientToGravity(ToSphereGravity, DeltaTime, 1.f);
	}
}
void ABasePawnPlayer::SetContactedWith(bool bIsContactedWith)
{
	if(bContactedWithFloor || bContactedWithLevelSphere || bContactedWithSphereFloor)
	{
		bContactedWithFloor = bIsContactedWith;
		bContactedWithSphereFloor = bIsContactedWith;
		bContactedWithLevelSphere = bIsContactedWith;
		Capsule->SetLinearDamping(AirFriction);
	}
}


