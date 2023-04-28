// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawnPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VectorTypes.h"
#include "Chaos/SpatialAccelerationCollection.h"
#include "Components/BoxComponent.h"
#include "Gravity/Components/ShooterCombatComponent.h"
#include "Gravity/Components/ShooterHealthComponent.h"
#include "Gravity/Flooring/FloorBase.h"
#include "Gravity/Flooring/SphereFloorBase.h"
#include "Gravity/Sphere/GravitySphere.h"
#include "Gravity/Weapons/WeaponBase.h"
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
	SpringArm->SetIsReplicated(true);
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

	DOREPLIFETIME(ABasePawnPlayer, bIsMagnetized);
	DOREPLIFETIME(ABasePawnPlayer, SpringArmClientPitch);
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
}

void ABasePawnPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	PerformGravity(DeltaTime);
	FindSphere();
	SphereFloorContactedGravity(DeltaTime);
}

void ABasePawnPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* PlayerEnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// This calls the handler function on the tick when MyInputAction starts, such as when pressing an action button.
		PlayerEnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Move);
		PlayerEnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Look);
		PlayerEnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Jump);
		PlayerEnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Crouch);
		PlayerEnhancedInputComponent->BindAction(MagnetizeAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Magnetize);
		PlayerEnhancedInputComponent->BindAction(BoostDirectionAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::BoostWithDirection);
		PlayerEnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Boost);
		PlayerEnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Equip);
		PlayerEnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ABasePawnPlayer::Fire);
	}
}

void ABasePawnPlayer::Move(const FInputActionValue& ActionValue)
{
	const FVector Value = ActionValue.Get<FVector>();
	if(!HasAuthority())
	{
		Move_Internal(Value);
		FShooterMove MoveToSend;
		MoveToSend.MovementVector = Value;
		UE_LOG(LogTemp, Warning, TEXT("Client On Client: %s"), *MoveToSend.MovementVector.ToString());
		SendServerMove(MoveToSend);
	}
	else
	{
		Server_Move(Value);
	}
}

void ABasePawnPlayer::Server_Move_Implementation(FVector ActionValue)
{
	Move_Internal(ActionValue);
}

void ABasePawnPlayer::Move_Internal(FVector ActionValue)
{
	//Adds to ControlInputValue, which is used in PerformPlayerMovement
	//Add differences for forward/backwards/lateral movement and if we are in the air or not
	if(FloorStatus != FShooterFloorStatus::NoFloorContact) //if contacted with a floor
	{
		AddMovementInput(GetActorRightVector() * ActionValue.Y * LateralSpeed);
		if(ActionValue.X > 0.f)
		{
			AddMovementInput(GetActorForwardVector() * ActionValue.X * ForwardSpeed);
		}
		else if(ActionValue.X < 0.f)
		{
			AddMovementInput(GetActorForwardVector() * ActionValue.X * BackwardsSpeed);
		}
	}
	else
	{
		AddMovementInput(GetActorForwardVector() * ActionValue.X);
		AddMovementInput(GetActorRightVector() * ActionValue.Y);
		AddMovementInput(GetActorUpVector() * ActionValue.Z);
	}
	PerformPlayerMovement();
}

void ABasePawnPlayer::PerformPlayerMovement()
{
	if(FloorStatus == FShooterFloorStatus::NoFloorContact) //if not contacted with floor
	{
		Capsule->AddImpulse(ControlInputVector * AirSpeed);
		ConsumeMovementInputVector();
	}
	else if(FloorStatus != FShooterFloorStatus::NoFloorContact && bIsMagnetized)//if contacted with floor and magnetized
	{
		Capsule->AddImpulse(ControlInputVector * GroundSpeed);
		ConsumeMovementInputVector();
	}
}


void ABasePawnPlayer::Look(const FInputActionValue& ActionValue)
{
	const FVector2D Value = ActionValue.Get<FVector2D>();
	if(!HasAuthority())
	{
		Look_Internal(Value);
	}
	if(HasAuthority())
	{
		Server_Look(Value);
	}
}


void ABasePawnPlayer::Server_Look_Implementation(FVector2D ActionValue)
{
	Look_Internal(ActionValue);
}

void ABasePawnPlayer::Look_Internal(FVector2D ActionValue)
{
	if(static_cast<int>(FloorStatus) > 0) //0 is not contact, any other contact is 1+
		{
		//Have to Rotate the actor without controller because controller will only rotate in World coordinates causing the character to improperly add yaw and pitch
		AddActorLocalRotation(FRotator(0.f,ActionValue.X,0.f));
		}
	else
	{
		Capsule->AddTorqueInDegrees(GetActorUpVector() * AirRotationSpeed * ActionValue.X, NAME_None, true);
	}
	
	//Have to Rotate the SpringArm without controller because the controller will only rotate in World coordinates causing the character to improperly add yaw and pitch
	const float SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
	if(SpringArmPitch <= -75.f)
	{
		//this if is for how far to look down
		if(ActionValue.Y >= 0.f)
		{
			SpringArm->AddLocalRotation(FRotator(ActionValue.Y,0.f,0.f));
		}
		else if(ActionValue.Y <= 0.f && static_cast<int>(FloorStatus) == 0 && !bIsMagnetized)
		{
			Capsule->AddTorqueInRadians(GetActorRightVector() * AirForwardRollSpeed);
		}
	}
	else if(SpringArmPitch >= 70.f)
	{
		//this if is for how far to look up
		if(ActionValue.Y <= 0.f)
		{
			SpringArm->AddLocalRotation(FRotator(ActionValue.Y,0.f,0.f));
		}
		else if(ActionValue.Y >= 0.f && static_cast<int>(FloorStatus) == 0 && !bIsMagnetized)
		{
			Capsule->AddTorqueInRadians(GetActorRightVector() * -AirForwardRollSpeed);
		}
	}
	else
	{
		SpringArm->AddLocalRotation(FRotator(ActionValue.Y,0.f,0.f));
	}
}

void ABasePawnPlayer::Jump(const FInputActionValue& ActionValue)
{
	const float Value = ActionValue.Get<float>();
	if(!HasAuthority() && IsLocallyControlled())
	{
		Jump_Internal(Value);
	}
	if(HasAuthority() && IsLocallyControlled())
	{
		Server_Jump(Value);
	}
}


void ABasePawnPlayer::Server_Jump_Implementation(float ActionValue)
{
	Jump_Internal(ActionValue);
}

void ABasePawnPlayer::Jump_Internal(float ActionValue)
{
	if(static_cast<int>(FloorStatus) > 0 && bIsMagnetized)
	{
		SetFloorStatus(FShooterFloorStatus::NoFloorContact);
		Capsule->AddImpulse(GetActorUpVector() * JumpVelocity * ActionValue);
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
	if(!HasAuthority() && IsLocallyControlled())
	{
		Magnetize_Internal();
	}
	if(HasAuthority() && IsLocallyControlled())
	{
		Server_Magnetize();
	}
}


void ABasePawnPlayer::Server_Magnetize_Implementation()
{
	Magnetize_Internal();
}

void ABasePawnPlayer::Magnetize_Internal()
{
	if(static_cast<int>(FloorStatus) > 0)
	{
		Jump(0.25f);
	}
	
	bIsMagnetized = !bIsMagnetized;
	
	if(!bIsMagnetized)
	{
		ZeroOutCurrentGravity();
		SetFloorStatus(FShooterFloorStatus::NoFloorContact);
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

void ABasePawnPlayer::BoostWithDirection(const FInputActionValue& ActionValue)
{
	const FVector BoostDirection = ActionValue.Get<FVector>();
	if(!HasAuthority() && IsLocallyControlled())
	{
		Boost_Internal(BoostDirection);
	}
	if(HasAuthority() && IsLocallyControlled())
	{
		Server_Boost(BoostDirection);
	}
}

void ABasePawnPlayer::Boost(const FInputActionValue& ActionValue)
{
	if(!HasAuthority() && IsLocallyControlled())
	{
		Boost_Internal(FVector::ZeroVector);
	}
	if(HasAuthority() && IsLocallyControlled())
	{
		Server_Boost(FVector::ZeroVector);
	}
}

void ABasePawnPlayer::Server_Boost_Implementation(FVector BoostDirection)
{
	Boost_Internal(BoostDirection);
}

void ABasePawnPlayer::Boost_Internal(FVector BoostDirection)
{
	bCanBoost = BoostCount >= MaxBoosts ? false : true;
	if(bCanBoost)
	{
		if(FloorStatus == FShooterFloorStatus::NoFloorContact)
		{
			Capsule->SetPhysicsLinearVelocity(GetVelocity() / BoostCurrentVelocityReduction);
			BoostForce(BoostDirection);
		}
		else
		{
			Magnetize(1.f);
			BoostForce(BoostDirection);
			GetWorldTimerManager().SetTimer(MagnetizeDelayForBoost, this, &ABasePawnPlayer::ContactedFloorMagnetizeDelay, MagnetizeDelay);
		}
	}
}

void ABasePawnPlayer::BoostForce(const FVector BoostDirection)
{
	if(BoostDirection.Size() != 0.f)
	{
		const FVector NormalizedBoostDirection = BoostDirection.GetSafeNormal();
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
	Magnetize(1.f);
}

void ABasePawnPlayer::Equip(const FInputActionValue& ActionValue)
{
	
}

void ABasePawnPlayer::Fire(const FInputActionValue& ActionValue)
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->RequestFire(GetHitTarget());
	}
}

void ABasePawnPlayer::PerformGravity(float DeltaTime)
{
	if(bIsMagnetized && static_cast<int>(FloorStatus) == 0)
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
	if(AFloorBase* Floor = Cast<AFloorBase>(OtherActor))
	{
		if(FloorStatus == FShooterFloorStatus::NoFloorContact && Capsule && bIsMagnetized)	
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
					SetFloorStatus(FShooterFloorStatus::BaseFloorContact);
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
			SetFloorStatus(FShooterFloorStatus::SphereLevelContact);
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
		SetFloorStatus(FShooterFloorStatus::SphereFloorContact);
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
	if(FloorStatus == FShooterFloorStatus::SphereFloorContact && SphereContactedWith)
	{
		//Pull towards the center of sphere floor while we are contacted with the floor
		const FVector ToSphereGravity = (SphereContactedWith->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		Capsule->AddForce(ToSphereGravity * SphereFloorContactedForceCorrection);
		OrientToGravity(ToSphereGravity, DeltaTime, 1.f, bHaveAGravity);
	}
}

float ABasePawnPlayer::GetSpringArmPitch() const
{
	if(HasAuthority())
	{
		return SpringArm->GetRelativeRotation().Pitch;
	}
	else
	{
		return SpringArmClientPitch;
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

void ABasePawnPlayer::SetFloorStatus(FShooterFloorStatus InFloorStatus)
{
	FloorStatus = InFloorStatus;
	if(FloorStatus == FShooterFloorStatus::NoFloorContact)
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
	FShooterMove MoveRecieved = ClientMove;
	Move_Internal(MoveRecieved.MovementVector);
}

