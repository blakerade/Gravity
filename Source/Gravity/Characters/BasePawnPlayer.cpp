// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawnPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VectorTypes.h"
#include "Chaos/SpatialAccelerationCollection.h"
#include "Components/BoxComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Gravity/Components/ShooterCombatComponent.h"
#include "Gravity/Components/ShooterHealthComponent.h"
#include "Gravity/Flooring/FloorBase.h"
#include "Gravity/Flooring/SphereFloorBase.h"
#include "Gravity/Sphere/GravitySphere.h"
#include "Gravity/Weapons/WeaponBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ABasePawnPlayer::ABasePawnPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
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
	Combat = CreateDefaultSubobject<UShooterCombatComponent>(TEXT("CombatComponent"));
	Health = CreateDefaultSubobject<UShooterHealthComponent>(TEXT("HealthComponent"));

	//HitBoxes
	Head = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	Head->SetupAttachment(Skeleton, FName("Head"));
	Spine2 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine2"));
	Spine2->SetupAttachment(Skeleton, FName("Spine2"));
	Hips = CreateDefaultSubobject<UBoxComponent>(TEXT("Hips"));
	Hips->SetupAttachment(Skeleton, FName("Hips"));
	RightUpLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("RightUpLeg"));
	RightUpLeg->SetupAttachment(Skeleton, FName("RightUpLeg"));
	LeftUpLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftUpLeg"));
	LeftUpLeg->SetupAttachment(Skeleton, FName("LeftUpLeg"));
	RightLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("RightLeg"));
	RightLeg->SetupAttachment(Skeleton, FName("RightLeg"));
	LeftLeg = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftLeg"));
	LeftLeg->SetupAttachment(Skeleton, FName("LeftLeg"));
	RightFoot = CreateDefaultSubobject<UBoxComponent>(TEXT("RightFoot"));
	RightFoot->SetupAttachment(Skeleton, FName("RightFoot"));
	LeftFoot = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftFoot"));
	LeftFoot->SetupAttachment(Skeleton, FName("LeftFoot"));
	RightArm = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArm"));
	RightArm->SetupAttachment(Skeleton, FName("RightArm"));
	LeftArm = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArm"));
	LeftArm->SetupAttachment(Skeleton, FName("LeftArm"));
	RightForeArm = CreateDefaultSubobject<UBoxComponent>(TEXT("RightForeArm"));
	RightForeArm->SetupAttachment(Skeleton, FName("RightForeArm"));
	LeftForeArm = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftForeArm"));
	LeftForeArm->SetupAttachment(Skeleton, FName("LeftForeArm"));
	RightHand = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHand"));
	RightHand->SetupAttachment(Skeleton, FName("RightHand"));
	LeftHand = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHand"));
	LeftHand->SetupAttachment(Skeleton, FName("LeftHand"));
}


void ABasePawnPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePawnPlayer, StatusOnServer);
	DOREPLIFETIME_CONDITION(ABasePawnPlayer, bIsMagnetized, COND_SkipOwner);
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
			Subsystem->AddMappingContext(CharacterMovementMapping, 2.f);
			Subsystem->AddMappingContext(CharacterCombatMapping, 1.f);
		}
	}
	if(Capsule)
	{
		Capsule->OnComponentHit.AddDynamic(this, &ABasePawnPlayer::OnFloorHit);
	}
	OnTakeAnyDamage.AddDynamic(this, &ABasePawnPlayer::PassDamageToHealth);
	SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
	SpringArmYaw = SpringArm->GetRelativeRotation().Yaw;
}

void ABasePawnPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//current fixed time step of 60 per second
	AccumulatedDeltaTime += DeltaTime;
	if(AccumulatedDeltaTime >= FixedTimeStep)
	{
		ShooterMovement(FixedTimeStep);
		PerformGravity(FixedTimeStep);
		FindSphere();
		SphereFloorContactedGravity(FixedTimeStep);
	}
	UE_LOG(LogTemp, Warning, TEXT("%f"), LastVelocity.Size());
}

void ABasePawnPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::MovePressed);
		PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::LookActivated);
		PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::JumpPressed);
		PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Crouch);
		PlayerEnhancedInputComponent->BindAction(MagnetizeAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::MagnetizePressed);
		PlayerEnhancedInputComponent->BindAction(BoostDirectionAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::BoostPressed);
		PlayerEnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Equip);
		PlayerEnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::FirePressed);
	}
}


void ABasePawnPlayer::ShooterMovement(float DeltaTime)
{
	if(IsLocallyControlled())
	{
		//build the move to either execute or send to the server
		FShooterMove MoveToSend;
		BuildMovement(MoveToSend);
		BuildLook(MoveToSend);
		Jump(MoveToSend);
		Magnetized(MoveToSend);
		Boost(MoveToSend);

		AddActorWorldOffset(Movement_Internal(MoveToSend.MovementVector, DeltaTime));
		SpringArm->SetRelativeRotation(PitchLook_Internal(MoveToSend.PitchRotation, DeltaTime));
		AddActorLocalRotation(YawLook_Internal(MoveToSend.YawRotation, DeltaTime));
		AddActorLocalRotation(AddShooterSpin_Internal());
		Jump_Internal(MoveToSend.bJumped);
		Magnetize_Internal(MoveToSend.bMagnetized);
		Boost_Internal(MoveToSend.BoostDirection, MoveToSend.bBoost);
		if(GetWorld() && GetWorld()->GetGameState()) MoveToSend.GameTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		
		if(!HasAuthority())
		{
			UnacknowledgedMoves.Add(MoveToSend);
			SendServerMove(MoveToSend);
		}
		if(HasAuthority())
		{
			StatusOnServer.ShooterTransform = GetActorTransform();
			StatusOnServer.Velocity = GetVelocity();
			StatusOnServer.Torque = Capsule->GetPhysicsAngularVelocityInRadians();
			StatusOnServer.bMagnetized = bIsMagnetized;
		}
	}
	
}

void ABasePawnPlayer::MovePressed(const FInputActionValue& ActionValue)
{
	MoveVector = ActionValue.Get<FVector>();
}

void ABasePawnPlayer::BuildMovement(FShooterMove& OutMove)
{
	OutMove.MovementVector = MoveVector;
	MoveVector = FVector::ZeroVector;
}

FVector ABasePawnPlayer::Movement_Internal(const FVector ActionValue, float DeltaTime)
{
	TotalMovementInput(ActionValue, DeltaTime);
	const FVector MovementVelocity = CalculateMovementVelocity(DeltaTime);
	ConsumeMovementInputVector();
	return MovementVelocity;
}

void ABasePawnPlayer::TotalMovementInput(const FVector ActionValue, float DeltaTime)
{
	if(FloorStatus != EShooterFloorStatus::NoFloorContact) //if contacted with a floor
	{
		if(ActionValue.X > 0.f && ActionValue.Y == 0.f)
		{
			//if just going forward, go GroundForwardSpeed
			AddMovementInput(GetActorForwardVector() * ActionValue.X * GroundForwardSpeed * DeltaTime);
		}
		if(ActionValue.X > 0.f && ActionValue.Y != 0.f)
		{
			//if going forward and any lateral input, go a constant ForwardLateralSpeed
			AddMovementInput(GetActorRightVector() * ActionValue.Y * (GroundForwardLateralSpeed/2.f) * DeltaTime);
			AddMovementInput(GetActorForwardVector() * ActionValue.X * (GroundForwardLateralSpeed/2.f) * DeltaTime);
		}
		if(ActionValue.X == 0.f && ActionValue.Y != 0.f)
		{
			//if not going forward and any lateral input, go a constant GroundLateralSpeed
			AddMovementInput(GetActorRightVector() * ActionValue.Y * GroundLateralSpeed * DeltaTime);
		}
		if(ActionValue.X < 0.f)
		{
			//if going backwards at all, go the GroundBackwardSpeed
			AddMovementInput(GetActorRightVector() * ActionValue.Y * (GroundBackwardSpeed/2.f) * DeltaTime);
			AddMovementInput(GetActorForwardVector() * ActionValue.X * (GroundBackwardSpeed/2.f) * DeltaTime);
		}
	}
	else //if not contacted with a floor
	{
		AddMovementInput(GetActorForwardVector() * ActionValue.X * DeltaTime);
		AddMovementInput(GetActorRightVector() * ActionValue.Y * DeltaTime);
		AddMovementInput(GetActorUpVector() * ActionValue.Z * DeltaTime);
	}
}

FVector ABasePawnPlayer::CalculateMovementVelocity(float DeltaTime)
{
	if(FloorStatus != EShooterFloorStatus::NoFloorContact && bIsMagnetized) //if contacted with floor and magnetized
	{
		if(ControlInputVector.Size() == 0.f)
		{
			return LastVelocity = FMath::VInterpTo(LastVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
		}
		return LastVelocity = FMath::VInterpTo(LastVelocity, ControlInputVector, DeltaTime, AccelerationSpeed);
	}
	if(ControlInputVector.Size() == 0.f)
	{
		return LastVelocity;
	}
	return LastVelocity += (ControlInputVector * AirSpeed);
}

void ABasePawnPlayer::LookActivated(const FInputActionValue& ActionValue)
{
	PitchValue = ActionValue.Get<FVector2D>().Y;
	YawValue = ActionValue.Get<FVector2D>().X;
}

void ABasePawnPlayer::BuildLook(FShooterMove& OutMove)
{
	OutMove.YawRotation = YawValue;
	OutMove.PitchRotation = PitchValue;
	PitchValue = 0.f;
	YawValue = 0.f;
}

FRotator ABasePawnPlayer::PitchLook_Internal(float ActionValueY, float DeltaTime)
{
	SpringArmPitch = FMath::Clamp(SpringArmPitch + (ActionValueY * DeltaTime * SpringArmPitchSpeed), SpringArmPitchMin, SpringArmPitchMax);
	if(SpringArmPitch > (SpringArmPitchMax - 3.f) && FloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		ShooterSpin = EShooterSpin::BackFlip;
	}
	else if(SpringArmPitch < (SpringArmPitchMin + 3.f) && FloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		ShooterSpin = EShooterSpin::FrontFlip;
	}
	return FRotator(SpringArmPitch, SpringArmYaw, 0.f);
}

FRotator ABasePawnPlayer::AddShooterSpin_Internal() const
{
	switch (ShooterSpin)
	{
	case EShooterSpin::BackFlip:
		return FRotator(AirPitchSpeed, 0.f, 0.f);
	case EShooterSpin::FrontFlip:
		return FRotator(-AirPitchSpeed, 0.f, 0.f);
	default:
		return FRotator::ZeroRotator;
	}
}

FRotator ABasePawnPlayer::YawLook_Internal(float ActionValueX, float DeltaTime)
{
	//We are contacted to a floor
	if(FloorStatus != EShooterFloorStatus::NoFloorContact)
	{
		return FRotator(0.f, ActionValueX, 0.f);
	}
	//We are not contacted to a floor
	if(ActionValueX == 0.f)
	{
		return FRotator(0.f, LastYawRotation, 0.f);
	}
	const float NewYawRotation = FMath::Clamp(LastYawRotation + ActionValueX * AirRotationSpeed * DeltaTime, -AirRotationMaxSpeed, AirRotationMaxSpeed);
	return FRotator(0.f, LastYawRotation = NewYawRotation, 0.f);
}

void ABasePawnPlayer::JumpPressed(const FInputActionValue& ActionValue)
{
	bJumpPressed = true;
}

void ABasePawnPlayer::Jump(FShooterMove& OutMove)
{
	if(bJumpPressed)
	{
		OutMove.bJumped = true;
		bJumpPressed = false;
	}
}

void ABasePawnPlayer::Jump_Internal(bool bJumpWasPressed)
{
	if(bJumpWasPressed)
	{
		if(FloorStatus != EShooterFloorStatus::NoFloorContact && bIsMagnetized) //if we are in contact with a floor
		{
			SetFloorStatus(EShooterFloorStatus::NoFloorContact);
			Capsule->AddImpulse(GetActorUpVector() * JumpVelocity);
			Capsule->SetLinearDamping(AirFriction);
		}
	}
}

void ABasePawnPlayer::Crouch(const FInputActionValue& ActionValue)
{
	
}

void ABasePawnPlayer::MagnetizePressed(const FInputActionValue& ActionValue)
{
	bMagnetizedPressed = true;
}


void ABasePawnPlayer::Magnetized(FShooterMove& OutMove)
{
	if(bMagnetizedPressed)
	{
		OutMove.bMagnetized = true;
		bMagnetizedPressed = false;
	}
}

void ABasePawnPlayer::Magnetize_Internal(bool bMagnetizedWasPressed)
{
	if(bMagnetizedWasPressed)
	{
		// Maybe put back in a way for a tiny jump if we unmagnetize
		// if(static_cast<int>(FloorStatus) > 0)
		// {
		// 	Jump(0.25f);
		// }
	
		bIsMagnetized = !bIsMagnetized;
	
		if(!bIsMagnetized)
		{
			ZeroOutCurrentGravity();
			SetFloorStatus(EShooterFloorStatus::NoFloorContact);
		}
		else if(FloorGravities.Num() != 0 || SphereFloors.Num() != 0)
		{
			if(Capsule)
			{
				Capsule->SetAllPhysicsLinearVelocity(GetVelocity()/2.f);
				Capsule->SetAllPhysicsAngularVelocityInRadians(Capsule->GetPhysicsAngularVelocityInRadians()/2.f);
			}
		}
	}
}

void ABasePawnPlayer::BoostPressed(const FInputActionValue& ActionValue)
{
	bBoostPressed = true;
	BoostDirection = ActionValue.Get<FVector>();
}

void ABasePawnPlayer::Boost(FShooterMove& OutMove)
{
	if(bBoostPressed)
	{
		OutMove.BoostDirection = BoostDirection;
		OutMove.bBoost = true;
		bBoostPressed = false;
		BoostDirection = FVector::ZeroVector;
	}
}

void ABasePawnPlayer::Boost_Internal(FVector ActionValue, bool bBoostWasPressed)
{
	if(bBoostWasPressed)
	{
		bCanBoost = BoostCount >= MaxBoosts ? false : true;
		if(bCanBoost)
		{
			if(FloorStatus == EShooterFloorStatus::NoFloorContact)
			{
				Capsule->SetPhysicsLinearVelocity(GetVelocity() / BoostCurrentVelocityReduction);
				BoostForce(ActionValue);
			}
			else
			{
				MagnetizePressed(1.f);
				BoostForce(ActionValue);
				GetWorldTimerManager().SetTimer(MagnetizeDelayForBoost, this, &ABasePawnPlayer::ContactedFloorMagnetizeDelay, MagnetizeDelay);
			}
		}
	}
}

void ABasePawnPlayer::BoostForce(const FVector ActionValue)
{
	if(ActionValue.Size() != 0.f)
	{
		const FVector NormalizedBoostDirection = ActionValue.GetSafeNormal();
		const FVector BoostVector((GetActorForwardVector() * NormalizedBoostDirection.X) +
			(GetActorRightVector() * NormalizedBoostDirection.Y) +
			(GetActorUpVector() * NormalizedBoostDirection.Z));
		Capsule->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
		Capsule->SetAllPhysicsLinearVelocity(BoostVector.GetSafeNormal() * BoostSpeed);
		BoostCount++;
		if(BoostCount >= MaxBoosts)
		{
			return;
		}
		else
		{
			GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &ABasePawnPlayer::BoostCountConsumer, BoostRechargeRate);
		}
	}
	else
	{
		Capsule->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
		Capsule->SetAllPhysicsLinearVelocity(GetVelocity().GetSafeNormal() * BoostSpeed);
		BoostCount++;
		if(BoostCount >= MaxBoosts)
		{
			return;
		}
		else
		{
			GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &ABasePawnPlayer::BoostCountConsumer, BoostRechargeRate);
		}
	}
}

void ABasePawnPlayer::BoostCountConsumer()
{
	BoostCount--;
	if(BoostCount > 0)
	{
		GetWorldTimerManager().SetTimer(BoostTimerHandle, this, &ABasePawnPlayer::BoostCountConsumer, BoostRechargeRate);
	}
}

void ABasePawnPlayer::ContactedFloorMagnetizeDelay()
{
	MagnetizePressed(1.f);
}

void ABasePawnPlayer::Equip(const FInputActionValue& ActionValue)
{
	
}

void ABasePawnPlayer::FirePressed(const FInputActionValue& ActionValue)
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->RequestFire(GetHitTarget());
	}
}

void ABasePawnPlayer::PerformGravity(float DeltaTime)
{
	if(bIsMagnetized && FloorStatus == EShooterFloorStatus::NoFloorContact)
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
			OrientToGravity(CurrentGravity, DeltaTime, GravityDistance, bHaveAGravity);
		}
		else if(bHaveAGravity && bIsASphereFloor)
		{
			//Pull towards the center of sphere floor
			Capsule->AddForce(CurrentGravity * SphereFloorGravityStrength);
			OrientToGravity(CurrentGravity, DeltaTime, GravityDistance, bHaveAGravity);
		}
		else if(bIsInsideSphere)
		{
			//Push us away from the center of the sphere
			const FVector AwayFromCenter = GetActorLocation() -SphereCenter;
			Capsule->AddForce(AwayFromCenter.GetSafeNormal() * SphereGravityStrength);
			OrientToGravity(AwayFromCenter, DeltaTime, GravityDistance, bHaveAGravity);
		}
	}
}

void ABasePawnPlayer::FindClosestGravity(float& OutDistanceToGravity, bool& OutIsAFloorBase)
{
	//Have any gravity triggers added themselves to the gravity array
	if(FloorGravities.Num() > 0)
	{
		FVector ClosestGravity;
		float DistanceToClosestGravity = 0;
		bool bHaveAHit = false;
		const UWorld* World = GetWorld();
		for(AFloorBase* GravityToCheck: FloorGravities)
		{
			if(World)
			{
				//Reach out in each floors gravity to find which one is the closest
				FHitResult HitResult;
				float FeetLocation = Capsule->GetScaledCapsuleHalfHeight();
				FVector FeetVector = GetActorLocation() - (GetActorUpVector() * FeetLocation);
				World->LineTraceSingleByChannel(HitResult, FeetVector, FeetVector +  (GravityToCheck->GetFloorGravity()), ECC_GameTraceChannel1);
				DrawDebugLine(World, FeetVector, FeetVector +(GravityToCheck->GetFloorGravity()), FColor::Red, false, 3.f);
				DrawDebugPoint(World, FeetVector, 24.f, FColor::Green, false, 3.f);
				if(HitResult.bBlockingHit)
				{
					//Check to make sure we dont have a non-gravity floor in the way
					if(AFloorBase* HitActor = Cast<AFloorBase>(HitResult.GetActor()))
					{
						//If we already have a gravity set as the current closest
						if(bHaveAHit)
						{
							//Check if this new hit is closer than the current closest gravity and set it as the new closest gravity if so
							if(HitResult.Distance < DistanceToClosestGravity)
							{
								ClosestGravity = GravityToCheck->GetFloorGravity();
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
							ClosestGravity = GravityToCheck->GetFloorGravity();
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

void ABasePawnPlayer::OrientToGravity(FVector GravityToOrientTo, float DeltaTime, float DistanceToGravity,  bool bIsThereAFloor)
{
	if(bIsThereAFloor)
	{
		//If we are going the same way as gravity, use MakeFromXY to reduce amount of unnecessary pivoting, could probably use even more improvement
		if(FVector::DotProduct(GetVelocity(), GravityToOrientTo) >= 1.f)
		{
			FeetToGravity = FRotationMatrix::MakeFromZY(-GravityToOrientTo, GetActorRightVector());
			FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), FeetToGravity.Rotator(), DeltaTime, 2.f);
			FQuat NewRotation = FQuat::Slerp(GetActorRotation().Quaternion(),FeetToGravity.ToQuat(), DeltaTime * 2.f);
			SetActorRotation(NewRotation);
		}
		else
		{
			//If gravity is any other direction then this MakeFromZX should give us the smoothest rotation
			FeetToGravity = FRotationMatrix::MakeFromZX(-GravityToOrientTo, GetActorForwardVector());
			FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), FeetToGravity.Rotator(), DeltaTime, 2.f);
			FQuat NewRotation = FQuat::Slerp(GetActorRotation().Quaternion(),FeetToGravity.ToQuat(), DeltaTime * 2.f);
			SetActorRotation(NewRotation);
			
		}
	}
	else
	{
		//If we are going the same way as gravity, use MakeFromXY to reduce amount of unnecessary pivoting, could probably use even more improvement
		if(FVector::DotProduct(GetVelocity(), GravityToOrientTo) >= 1.f)
		{
			FeetToGravity = FRotationMatrix::MakeFromZY(-GravityToOrientTo, GetActorRightVector());
			FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), FeetToGravity.Rotator(), DeltaTime, 0.15f);
			SetActorRotation(InterpRotation);
		}
		else
		{
			//If gravity is any other direction then this MakeFromZX should give us the smoothest rotation
			FeetToGravity = FRotationMatrix::MakeFromZX(-GravityToOrientTo, GetActorForwardVector());
			FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), FeetToGravity.Rotator(), DeltaTime, 0.15f);
			SetActorRotation(InterpRotation);
		}
	}

}

void ABasePawnPlayer::OnFloorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(FloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		if(AFloorBase* Floor = Cast<AFloorBase>(OtherActor))
		{
			ShooterSpin = EShooterSpin::NoFlip;
			if(Capsule && bIsMagnetized)	
			{
				if(Floor->GetFloorGravity() == CurrentGravity || Floor->GetFloorGravity() == -CurrentGravity)
				{
					if(FVector::DotProduct(-GetActorUpVector(), CurrentGravity.GetSafeNormal()) < 0.7f)
					{
						MagnetizePressed(1.f);
						ZeroOutCurrentGravity();
					}
					else
					{
						LastVelocity = FVector::ZeroVector;
						ConsumeMovementInputVector();
						LastYawRotation = 0.f;
						//If the Rinterpto isn't done we still need to be rotated corrected when we land
						SetActorRotation(FeetToGravity.Rotator());
						//Important bool for other functionality
						SetFloorStatus(EShooterFloorStatus::BaseFloorContact);
						//Stops the capsule from falling over
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
				SetFloorStatus(EShooterFloorStatus::SphereLevelContact);
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
			SetFloorStatus(EShooterFloorStatus::SphereFloorContact);
			//Stops the capsule from falling over
			Capsule->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			//Set damping to a high value so that when we are walking it doesn't feel like we are skating
			Capsule->SetLinearDamping(FloorFriction);
			//Set what sphere floor with are contacted with so that we can continue to be pulled and oriented to it
			SphereContactedWith = SphereFloor;
		}
	}
}

void ABasePawnPlayer::SphereFloorContactedGravity(float DeltaTime)
{
	if(FloorStatus == EShooterFloorStatus::SphereFloorContact && SphereContactedWith)
	{
		//Pull towards the center of sphere floor while we are contacted with the floor
		const FVector ToSphereGravity = (SphereContactedWith->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		Capsule->AddForce(ToSphereGravity * SphereFloorContactedForceCorrection);
		OrientToGravity(ToSphereGravity, DeltaTime, 1.f, bHaveAGravity);
	}
}

FVector ABasePawnPlayer::GetHitTarget()
{
	if(Combat)
	{
		return Combat->HitTarget;
	}
	else
	{
		return FVector::ZeroVector;
	}
}

void ABasePawnPlayer::PassDamageToHealth(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	Health->TakeDamage(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
}

void ABasePawnPlayer::SetFloorStatus(EShooterFloorStatus InFloorStatus)
{
	FloorStatus = InFloorStatus;
	if(FloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		Capsule->SetLinearDamping(AirFriction);
	}
	else
	{
		Capsule->SetLinearDamping(FloorFriction);
	}
}

void ABasePawnPlayer::SendServerMove_Implementation(FShooterMove ClientMove)
{
	Movement_Internal(ClientMove.MovementVector, FixedTimeStep);
	// Look_Internal(ClientMove.PitchVector);
	Jump_Internal(ClientMove.bJumped);
	Magnetize_Internal(ClientMove.bMagnetized);
	Boost_Internal(ClientMove.BoostDirection, ClientMove.bBoost);
	StatusOnServer.ShooterTransform = GetActorTransform();
	StatusOnServer.Velocity = GetVelocity();
	StatusOnServer.Torque = Capsule->GetPhysicsAngularVelocityInRadians();
	StatusOnServer.bMagnetized = bIsMagnetized;
	StatusOnServer.LastMove = ClientMove;
}

void ABasePawnPlayer::OnRep_StatusOnServer()
{
	SetActorTransform(StatusOnServer.ShooterTransform);
	Capsule->SetPhysicsLinearVelocity(StatusOnServer.Velocity);
	Capsule->SetPhysicsAngularVelocityInRadians(StatusOnServer.Torque);
	bIsMagnetized = StatusOnServer.bMagnetized;
	ClearAcknowledgedMoves();
	PlayUnacknowledgedMoves();
}

void ABasePawnPlayer::ClearAcknowledgedMoves()
{
	TArray<FShooterMove> NewMoves;
	for(FShooterMove MoveToCheck : UnacknowledgedMoves)
	{
		if(StatusOnServer.LastMove.GameTime < MoveToCheck.GameTime)
		{
			NewMoves.Add(MoveToCheck);
		}
	}
	UnacknowledgedMoves = NewMoves;
}

void ABasePawnPlayer::PlayUnacknowledgedMoves()
{
	for(const FShooterMove MoveToPlay: UnacknowledgedMoves)
	{
		Movement_Internal(MoveToPlay.MovementVector, FixedTimeStep);
		Jump_Internal(MoveToPlay.bJumped);
		Magnetize_Internal(MoveToPlay.bMagnetized);
		Boost_Internal(MoveToPlay.BoostDirection, MoveToPlay.bBoost);
	}
}
