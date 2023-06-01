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
#include "Kismet/KismetMathLibrary.h"
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
}

void ABasePawnPlayer::BeginPlay()
{
	Super::BeginPlay();

	// Make sure that we have a valid PlayerController.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// Get the Enhanced Input Local Player Subsystem from the Local Player related to our Player Controller.
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// PawnClientRestart can run more than once in an Actor's lifetime, so start by clearing out any leftover mappings.
			Subsystem->ClearAllMappings();

			// Add each mapping context, along with their priority values. Higher values out prioritize lower values.
			Subsystem->AddMappingContext(CharacterMovementMapping, 2.f);
			Subsystem->AddMappingContext(CharacterCombatMapping, 1.f);
		}
	}
	if(Capsule)
	{
		Capsule->OnComponentHit.AddDynamic(this, &ABasePawnPlayer::OnFloorHit);
	}
	OnTakeAnyDamage.AddDynamic(this, &ABasePawnPlayer::PassDamageToHealth);
	if(IsLocallyControlled())
	{
		LocalStatus.SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
		LocalStatus.SpringArmYaw = SpringArm->GetRelativeRotation().Yaw;
		LocalStatus.ShooterRotation = GetActorRotation();
		LocalStatus.bMagnetized = false;
		//TODO all this stuff
		
	}
	else
	{
		StatusOnServer.ShooterLocation = GetActorLocation();
		StatusOnServer.SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
		StatusOnServer.SpringArmYaw = SpringArm->GetRelativeRotation().Yaw;
		StatusOnServer.ShooterFloorStatus = EShooterFloorStatus::NoFloorContact;
		StatusOnServer.ShooterSpin = EShooterSpin::NoFlip;
	}
}

void ABasePawnPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//current fixed time step of 60 per second
	AccumulatedDeltaTime += DeltaTime;
	if(AccumulatedDeltaTime >= FixedTimeStep)
	{
		ShooterMovement(FixedTimeStep);
		InterpAutonomousCSPTransform(FixedTimeStep);
		MoveClientProxies(FixedTimeStep);
		DebugMode();
	}
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
		FShooterMove MoveToSend;
		if(!bIsInterpolatingClientStatus)
		{
			//build the move to either execute or send to the server and the server is not fixing our position
			
			BuildMovement(MoveToSend);
			BuildJump(MoveToSend);
			BuildMagnetized(MoveToSend);
			BuildBoost(MoveToSend);
			MoveToSend.ShooterLocation = GetActorLocation();
			MoveToSend.ShooterRotationBeforeMovement = GetActorRotation();
			if(GetWorld() && GetWorld()->GetGameState()) MoveToSend.GameTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
			
			LocalStatus.CurrentVelocity += Movement_Internal(MoveToSend.MovementVector, GetActorLocation(), GetActorRotation(), LocalStatus.CurrentVelocity, DeltaTime);
			AddActorWorldOffset(LocalStatus.CurrentVelocity);
			Magnetize_Internal(MoveToSend.bMagnetizedPressed, LocalStatus);
			
			AddActorWorldOffset(Jump_Internal(MoveToSend.bJumped,GetActorTransform(), DeltaTime));
			
			Boost_Internal(MoveToSend.BoostDirection, MoveToSend.bBoost, GetActorTransform(), DeltaTime);
			SetActorTransform(PerformGravity(GetActorTransform(), DeltaTime));
		}
		SpringArm->SetRelativeRotation(PitchLook_Internal(LocalStatus.ShooterSpin, LocalStatus.SpringArmPitch,LocalStatus, DeltaTime));
		AddActorLocalRotation(AddShooterSpin_Internal(LocalStatus, DeltaTime));
		AddActorLocalRotation(YawLook_Internal(LocalStatus, DeltaTime));
		MoveToSend.LastPitchRotation = LocalStatus.LastPitchRotation;
		MoveToSend.LastYawRotation = LocalStatus.LastYawRotation;
		MoveToSend.ShooterRotationAfterMovement = GetActorRotation();
		MoveToSend.SpringArmPitch = LocalStatus.SpringArmPitch;
		
		if(!HasAuthority() && !bIsInterpolatingClientStatus)
		{
			UnacknowledgedMoves.Add(MoveToSend);
			ServerSendMove(MoveToSend);
		}
		if(HasAuthority())
		{
			StatusOnServer = LocalStatus;
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

FVector ABasePawnPlayer::Movement_Internal(const FVector ActionValue, FVector ActorLocation , FRotator ActorRotation, FVector LastVelocity, float DeltaTime)
{
	const FVector InputMovementVector = TotalMovementInput(ActionValue, ActorRotation, DeltaTime);
	return CalculateMovementVelocity(InputMovementVector, ActorLocation,  ActorRotation, LastVelocity, DeltaTime);
}

FVector ABasePawnPlayer::TotalMovementInput(const FVector ActionValue, FRotator ActorRotation, float DeltaTime) const
{
	const FVector ForwardVector = ActorRotation.Quaternion().GetAxisX();
	const FVector RightVector = ActorRotation.Quaternion().GetAxisY();
	const FVector UpVector = ActorRotation.Quaternion().GetAxisZ();

	if(FloorStatus != EShooterFloorStatus::NoFloorContact) //if contacted with a floor
	{
		
		if(ActionValue.X > 0.f && ActionValue.Y == 0.f)
		{
			//if just going forward, go GroundForwardSpeed
			return ForwardVector * ActionValue.X * GroundForwardSpeed * DeltaTime;
		}
		if(ActionValue.X > 0.f && ActionValue.Y != 0.f)
		{
			//if going forward and any lateral input, go a constant ForwardLateralSpeed
			return RightVector * ActionValue.Y * (GroundForwardLateralSpeed/2.f) * DeltaTime + ForwardVector * ActionValue.X * (GroundForwardLateralSpeed/2.f) * DeltaTime;
		}
		if(ActionValue.X == 0.f && ActionValue.Y != 0.f)
		{
			//if not going forward and any lateral input, go a constant GroundLateralSpeed
			return RightVector * ActionValue.Y * GroundLateralSpeed * DeltaTime;
		}
		if(ActionValue.X < 0.f)
		{
			//if going backwards at all, go the GroundBackwardSpeed
			return RightVector * ActionValue.Y * (GroundBackwardSpeed/2.f) * DeltaTime + ForwardVector * ActionValue.X * (GroundBackwardSpeed/2.f) * DeltaTime;
		}
		
	}
	else //if not contacted with a floor
	{
		return ForwardVector * ActionValue.X * DeltaTime + RightVector * ActionValue.Y * DeltaTime +  UpVector * ActionValue.Z * DeltaTime;
	}
	return FVector::ZeroVector;
}

FVector ABasePawnPlayer::CalculateMovementVelocity(FVector InMovementInput, FVector ActorLocation, FRotator ActorRotation, FVector LastVelocity, float DeltaTime)
{
	if(FloorStatus == EShooterFloorStatus::BaseFloorContact && bIsMagnetized) //if walking on a flat floor and magnetized
	{
		if(InMovementInput.Size() == 0.f)
		{
			return FMath::VInterpTo(LastVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
		}
		return  FMath::VInterpTo(LastVelocity, InMovementInput, DeltaTime, AccelerationSpeed);
	}
	if(FloorStatus == EShooterFloorStatus::SphereFloorContact && bIsMagnetized) //if walking on a sphere and magnetized
	{
		if(InMovementInput.Size() == 0.f)
		{
			SphereLastVelocity = FMath::VInterpTo(SphereLastVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
			const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(SphereLastVelocity, ActorRotation.Quaternion().GetAxisZ());
			const FVector SphereToActor = ActorLocation - SphereLocation;
			const FVector NewPosition = SphereToActor.RotateAngleAxis(SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
			return NewPosition - SphereToActor;
		}
		const FVector AdjustedControlInputVector = InMovementInput * SphereFloorMovementPercent;
		SphereLastVelocity = FMath::VInterpTo(SphereLastVelocity, AdjustedControlInputVector, DeltaTime, AccelerationSpeed);
		const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(SphereLastVelocity, ActorRotation.Quaternion().GetAxisZ());
		const FVector SphereToActor = ActorLocation - SphereLocation;
		const FVector NewPosition = SphereToActor.RotateAngleAxis(SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
		return NewPosition - SphereToActor;
	}
	if(FloorStatus == EShooterFloorStatus::SphereLevelContact && bIsMagnetized) //if walking in a sphere and magnetized OR jumping on level sphere
	{
		if(InMovementInput.Size() == 0.f)
		{
			SphereLastVelocity = FMath::VInterpTo(SphereLastVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
			const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(SphereLastVelocity, ActorRotation.Quaternion().GetAxisZ());
			const FVector SphereToActor = ActorLocation - SphereLocation;
			const FVector NewPosition = SphereToActor.RotateAngleAxis(SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
			return NewPosition - SphereToActor;
		}
		const FVector AdjustedControlInputVector = -InMovementInput * LevelSphereMovementPercent;
		SphereLastVelocity = FMath::VInterpTo(SphereLastVelocity, AdjustedControlInputVector, DeltaTime, AccelerationSpeed);
		const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(SphereLastVelocity, ActorRotation.Quaternion().GetAxisZ());
		const FVector SphereToActor = ActorLocation - SphereLocation;
		const FVector NewPosition = SphereToActor.RotateAngleAxis(SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
		return NewPosition - SphereToActor;
	}
	return InMovementInput * AirSpeed;
}

void ABasePawnPlayer::LookActivated(const FInputActionValue& ActionValue)
{
	PitchValue = ActionValue.Get<FVector2D>().Y;
	YawValue = ActionValue.Get<FVector2D>().X;
}

FRotator ABasePawnPlayer::PitchLook_Internal(EShooterSpin& OutShooterSpin, float& OutSpringArmPitch, FShooterStatus InStatus, float DeltaTime) const
{
	OutSpringArmPitch = FMath::Clamp(OutSpringArmPitch + (PitchValue * DeltaTime * SpringArmPitchSpeed), SpringArmPitchMin, SpringArmPitchMax);
	if(OutSpringArmPitch > (SpringArmPitchMax - 1.5f) && InStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		OutShooterSpin = EShooterSpin::BackFlip;
	}
	else if(OutSpringArmPitch < (SpringArmPitchMin + 1.5f) && InStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		OutShooterSpin = EShooterSpin::FrontFlip;
	}
	else
	{
		OutShooterSpin = EShooterSpin::NoFlip;
	}
	return FRotator(OutSpringArmPitch, InStatus.SpringArmYaw, 0.f);
}

FRotator ABasePawnPlayer::AddShooterSpin_Internal(FShooterStatus InStatus, float DeltaTime)
{
	//We are not contacted to a floor
	if(InStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		float NewPitchRotation;
		switch (InStatus.ShooterSpin)
		{
		case EShooterSpin::BackFlip:
			if(PitchValue <= 0.f)
			{
				return FRotator(LocalStatus.LastPitchRotation, 0.f, 0.f);
			}
			NewPitchRotation = FMath::Clamp(LocalStatus.LastPitchRotation + PitchValue * AirPitchSpeed * DeltaTime, -MaxPitchSpeed, MaxPitchSpeed);
			return FRotator(LocalStatus.LastPitchRotation = NewPitchRotation, 0.f, 0.f);
		case EShooterSpin::FrontFlip:
			if(PitchValue >= 0.f)
			{
				return FRotator(LocalStatus.LastPitchRotation, 0.f, 0.f);
			}
			NewPitchRotation = FMath::Clamp(LocalStatus.LastPitchRotation - PitchValue * -AirPitchSpeed * DeltaTime, -MaxPitchSpeed, MaxPitchSpeed);
			return FRotator(LocalStatus.LastPitchRotation = NewPitchRotation, 0.f, 0.f);
		case EShooterSpin::NoFlip:
			return FRotator(LocalStatus.LastPitchRotation, 0.f, 0.f);
		default:
			return FRotator(LocalStatus.LastPitchRotation, 0.f, 0.f);
		}
	}
	return FRotator(LocalStatus.LastPitchRotation, 0.f, 0.f);
}

FRotator ABasePawnPlayer::YawLook_Internal(FShooterStatus InStatus, float DeltaTime)
{
	//We are contacted to a floor
	if(InStatus.ShooterFloorStatus != EShooterFloorStatus::NoFloorContact)
	{
		return FRotator(0.f, YawValue, 0.f);
	}
	//We are not contacted to a floor
	if(YawValue == 0.f)
	{
		return FRotator(0.f, LocalStatus.LastYawRotation, 0.f);
	}
	const float NewYawRotation = FMath::Clamp(LocalStatus.LastYawRotation + YawValue * AirRotationSpeed * DeltaTime, -AirRotationMaxSpeed, AirRotationMaxSpeed);
	return FRotator(0.f, LocalStatus.LastYawRotation = NewYawRotation, 0.f);
}

void ABasePawnPlayer::JumpPressed(const FInputActionValue& ActionValue)
{
	bJumpPressed = true;
}

void ABasePawnPlayer::BuildJump(FShooterMove& OutMove)
{
	if(bJumpPressed)
	{
		OutMove.bJumped = true;
		bJumpPressed = false;
	}
}

FVector ABasePawnPlayer::Jump_Internal(bool bJumpWasPressed, FTransform ActorTransform, float DeltaTime)
{
	if(!FMath::IsNearlyZero(JumpForce.Size()) && bIsMagnetized)
	{
		JumpForce = FMath::VInterpTo(JumpForce, FVector::ZeroVector, DeltaTime, JumpDeceleration);
		CurrentJumpVelocity = ActorTransform.GetLocation() - LastJumpPosition;
		LastJumpPosition = ActorTransform.GetLocation();
		return JumpForce;

	}
	if(bJumpWasPressed)
	{
		if(FloorStatus != EShooterFloorStatus::NoFloorContact && bIsMagnetized) //if we are in contact with a floor
		{
			if(FloorStatus == EShooterFloorStatus::SphereLevelContact)
			{
				bIsJumpingOffSphereLevel = true;
			}
			// SetFloorStatus(EShooterFloorStatus::NoFloorContact); TODO
			JumpForce = ActorTransform.GetUnitAxis(EAxis::Z) * JumpVelocity;
			// JumpForce += LastVelocity; TODO
			LastJumpPosition = ActorTransform.GetLocation();
			return JumpForce;
		}
	}
	return JumpForce = FVector::ZeroVector;
}

void ABasePawnPlayer::Crouch(const FInputActionValue& ActionValue)
{
	//ideas for crouch
	//don't give the ability to shoot when crouched, stops rapid crouching during fights
	//give the ability for the camera to look around, possibly increasing sa when taking away ability to fight
}

void ABasePawnPlayer::MagnetizePressed(const FInputActionValue& ActionValue)
{
	bMagnetizedPressed = true;
}


void ABasePawnPlayer::BuildMagnetized(FShooterMove& OutMove)
{
	if(bMagnetizedPressed)
	{
		OutMove.bMagnetizedPressed = true;
		bMagnetizedPressed = false;
	}
}

void ABasePawnPlayer::Magnetize_Internal(bool bMagnetizedFromMove, FShooterStatus& OutStatus)
{
	if(bMagnetizedFromMove)
	{
		OutStatus.bMagnetized = !OutStatus.bMagnetized;
	}
	if(!OutStatus.bMagnetized)
	{
		OutStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::NoFloorContact, OutStatus);
		// if(CurrentJumpVelocity.Size() > 0) TODO fix jumping
		// {
		// 	// LastVelocity = CurrentJumpVelocity; TODO
		// 	if(bIsJumpingOffSphereLevel)
		// 	{
		// 		SphereLastVelocity = CurrentJumpVelocity;
		// 		bIsJumpingOffSphereLevel = false;
		// 	}
		// 	CurrentJumpVelocity = FVector::ZeroVector;
		// }
	}
}

void ABasePawnPlayer::BoostPressed(const FInputActionValue& ActionValue)
{
	bBoostPressed = true;
	BoostDirection = ActionValue.Get<FVector>();
}

void ABasePawnPlayer::BuildBoost(FShooterMove& OutMove)
{
	if(bBoostPressed)
	{
		OutMove.BoostDirection = BoostDirection;
		OutMove.bBoost = true;
		bBoostPressed = false;
		BoostDirection = FVector::ZeroVector;
	}
}

void ABasePawnPlayer::Boost_Internal(FVector BoostVector, bool bBoostWasPressed, FTransform InActorTransform, float DeltaTime)
{
	if(bBoostWasPressed)
	{
		if(BoostCount > 0)
		{
			BoostCount --;
			if(FloorStatus == EShooterFloorStatus::NoFloorContact)
			{
				const FVector WorldBoostVector = InActorTransform.TransformVectorNoScale(BoostVector);
				// LastVelocity = WorldBoostVector * NonContactedBoostSpeed + LastVelocity /= BoostLastVelocityReduction; TODO
				GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRecharge, BoostRechargeRate);
			}
			else
			{
				ContactedBoostForce(BoostVector, InActorTransform, DeltaTime);
				GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRecharge, BoostRechargeRate);
			}
		}
	}
}

void ABasePawnPlayer::ContactedBoostForce(const FVector BoostVector, FTransform InActorTransform, float DeltaTime)
{
	const FVector WorldBoostVector = ContactedBoostSpeed * InActorTransform.TransformVectorNoScale(BoostVector);
	if(FloorStatus == EShooterFloorStatus::BaseFloorContact)
	{
		// LastVelocity = WorldBoostVector; TODO
	}
	if(FloorStatus == EShooterFloorStatus::SphereFloorContact)
	{
		SphereLastVelocity = WorldBoostVector * SphereFloorMovementPercent;
	}
	if(FloorStatus == EShooterFloorStatus::SphereLevelContact)
	{
		SphereLastVelocity = -WorldBoostVector * LevelSphereMovementPercent;
	}
}

void ABasePawnPlayer::BoostRecharge()
{
	BoostCount++;
	if(BoostCount < MaxBoosts)
	{
		GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRecharge, BoostRechargeRate);
	}
}

FTransform ABasePawnPlayer::PerformGravity(FTransform InActorTransform, float DeltaTime)
{
	FTransform NewActorTransform = InActorTransform;
	if(bIsMagnetized && FloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		if(bIsJumpingOffSphereLevel)
		{
			ClosestFloor = FindClosestFloor(NewActorTransform, CurrentGravity);
			if(ClosestFloor != nullptr)
			{
				SphereLocation = ClosestFloor->GetActorLocation();
				CurrentGravity = NewActorTransform.GetLocation() - SphereLocation;
				NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), DeltaTime).Quaternion());
				LastPitchRotation = 0.f;
				NewActorTransform.SetLocation(GravityForce(NewActorTransform.GetLocation(), DeltaTime));
			}
		}
		else
		{
			ClosestFloor = FindClosestFloor(NewActorTransform, CurrentGravity);
			if(ClosestFloor != nullptr)
			{
				NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), DeltaTime).Quaternion());
				LastPitchRotation = 0.f;
				NewActorTransform.SetLocation(GravityForce(NewActorTransform.GetLocation(), DeltaTime));
			}
		}
		return NewActorTransform;
	}
	if(bIsMagnetized && FloorStatus == EShooterFloorStatus::SphereFloorContact)
	{
		if(ClosestFloor != nullptr)
		{
			SphereLocation = ClosestFloor->GetActorLocation();
		}
		CurrentGravity = SphereLocation - NewActorTransform.GetLocation();
		NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), DeltaTime).Quaternion());
		LastPitchRotation = 0.f;
		return NewActorTransform;
	}
	if(bIsMagnetized && FloorStatus == EShooterFloorStatus::SphereLevelContact)
	{
		if(ClosestFloor != nullptr)
		{
			SphereLocation = ClosestFloor->GetActorLocation();
		}
		if(bIsJumpingOffSphereLevel)
		{
			NewActorTransform.SetLocation(GravityForce(NewActorTransform.GetLocation(), DeltaTime));
		}
		CurrentGravity = NewActorTransform.GetLocation() - SphereLocation;
		NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), DeltaTime).Quaternion());
		LastPitchRotation = 0.f;
		return NewActorTransform;
	}
	return InActorTransform;
}

AActor* ABasePawnPlayer::FindClosestFloor(FTransform ActorTransform, FVector& OutGravityDirection)
{
	//although feet makes more sense for magnetized boots, head position plays more predictably
	TArray<AActor*> OutActors;
	if(UWorld* World = GetWorld())
	{
		TArray<FOverlapResult> HitOverlaps;
		FCollisionQueryParams QueryParams;
		FCollisionResponseParams ResponseParams;
		FCollisionShape GravitySphere = FCollisionShape::MakeSphere(GravityDistanceRadius);
		FCollisionShape TraceShape = FCollisionShape::MakeSphere(SphereTraceRadius);
		
		World->OverlapMultiByChannel(HitOverlaps, ActorTransform.GetLocation(), FQuat::Identity, ECC_GameTraceChannel1, GravitySphere, QueryParams, ResponseParams);
		if(bIsInDebugMode)
		{
			DrawDebugSphere(World, ActorTransform.GetLocation(), GravityDistanceRadius, 32.f, FColor::Green);
		}
		//use a sphere trace to hit a part of the floor that is closer to the player than the center
		for(FOverlapResult Floor : HitOverlaps)
		{
			if(World && Floor.GetActor())
			{
				World->SweepSingleByChannel(FloorHitResult, ActorTransform.GetLocation(), Floor.GetActor()->GetActorLocation(), FQuat::Identity, ECC_GameTraceChannel1, TraceShape, QueryParams, ResponseParams);
				if(FloorHitResult.bBlockingHit && (FloorHitResult.ImpactPoint - ActorTransform.GetLocation()).Size() < ClosestDistanceToFloor)
				{
					if(bIsInDebugMode)
					{
						DrawDebugPoint(World, FloorHitResult.ImpactPoint, 50.f, FColor::Red);
					}
					ClosestDistanceToFloor = (FloorHitResult.ImpactPoint - ActorTransform.GetLocation()).Size();
					ClosestFloor = Floor.GetActor();
					OutGravityDirection = FloorHitResult.ImpactPoint - ActorTransform.GetLocation();
				}
			}
		}
	}
	return ClosestFloor;
}

FRotator ABasePawnPlayer::OrientToGravity(FRotator InActorRotator, float DeltaTime)
{
	FMatrix FeetToGravity;
	//If we are going the same way as gravity, use MakeFromXY to reduce amount of unnecessary pivoting, could probably use even more improvement
	// if(FVector::DotProduct(LastVelocity, CurrentGravity) >= 1.f)
	// {
	// 	FeetToGravity = FRotationMatrix::MakeFromZY(-CurrentGravity, GetActorRightVector());
	// 	const FQuat NewRotation = FQuat::Slerp(InActorRotator.Quaternion(),FeetToGravity.ToQuat(), DeltaTime * (SlerpSpeed/ClosestDistanceToFloor));
	//
	// 	return NewRotation.Rotator();
	// }
	//If gravity is any other direction then this MakeFromZX should give us the smoothest rotation
	FeetToGravity = FRotationMatrix::MakeFromZX(-CurrentGravity, GetActorForwardVector());
	FQuat NewRotation;
	if(FloorStatus == EShooterFloorStatus::SphereFloorContact || FloorStatus == EShooterFloorStatus::SphereLevelContact)
	{
		NewRotation = FQuat::Slerp(InActorRotator.Quaternion(),FeetToGravity.ToQuat(), 1.f);
	}
	else
	{
		NewRotation = FQuat::Slerp(InActorRotator.Quaternion(),FeetToGravity.ToQuat(), DeltaTime * (SlerpSpeed/ClosestDistanceToFloor));
	}
	return NewRotation.Rotator();
}

FVector ABasePawnPlayer::GravityForce(FVector InActorLocation, float DeltaTime)
{
	ClosestDistanceToFloor = FloorHitResult.Distance + SphereTraceRadius;
	const float DistancePct = FMath::Abs(150 - 100 * (ClosestDistanceToFloor/GravityDistanceRadius));
	FVector NewVector;
	if(FloorHitResult.bBlockingHit && DistancePct == 100.f)
	{
		NewVector = FMath::VInterpConstantTo(InActorLocation, FVector(FloorHitResult.ImpactPoint), DeltaTime, InRangeGravityStrength);
	}
	else if(FloorHitResult.bBlockingHit)
	{
		NewVector = FMath::VInterpConstantTo(InActorLocation, FVector(FloorHitResult.ImpactPoint), DeltaTime, FMath::Pow(OutRangeGravityStrength * DistancePct, GravityForceCurve));
	}
	// LastVelocity = FMath::VInterpTo(LastVelocity, FVector::ZeroVector, DeltaTime, GravityVelocityReduction); TODO
	SphereLastVelocity = FVector::ZeroVector;
	return NewVector;
}

void ABasePawnPlayer::OnFloorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(FloorStatus == EShooterFloorStatus::NoFloorContact && bIsMagnetized)
	{
		if(FVector::DotProduct(GetActorUpVector(), Hit.ImpactNormal) > 0.7f)
		{
			if(const ASphereFloorBase* SphereFloor = Cast<ASphereFloorBase>(OtherActor))
			{
				// SetFloorStatus(EShooterFloorStatus::SphereFloorContact); TODO
				ShooterSpin = EShooterSpin::NoFlip;
				LastPitchRotation = 0.f;
				LastYawRotation = 0.f;
				// LastVelocity = FVector::ZeroVector; TODO
				SphereLastVelocity = FVector::ZeroVector;
				SetActorRotation(FRotationMatrix::MakeFromZX(Hit.ImpactNormal, GetActorForwardVector()).Rotator());
			}
			else if(const AGravitySphere* LevelSphere = Cast<AGravitySphere>(OtherActor))
			{
				// SetFloorStatus(EShooterFloorStatus::SphereLevelContact); TODO
				ShooterSpin = EShooterSpin::NoFlip;
				LastPitchRotation = 0.f;
				LastYawRotation = 0.f;
				// LastVelocity = FVector::ZeroVector; TODO
				SphereLastVelocity = FVector::ZeroVector;
				SetActorRotation(FRotationMatrix::MakeFromZX(Hit.ImpactNormal, GetActorForwardVector()).Rotator());
			}
			else if(const AFloorBase* Floor = Cast<AFloorBase>(OtherActor))
			{
				const float DotProductResult = FVector::DotProduct(Floor->GetActorUpVector(), Hit.ImpactNormal);
				constexpr  float Epsilon = 0.001f;
				if(FMath::IsNearlyEqual(DotProductResult, 1.f, Epsilon) || FMath::IsNearlyEqual(DotProductResult, -1.f, Epsilon))
				{
					// SetFloorStatus(EShooterFloorStatus::BaseFloorContact); TODO
					ShooterSpin = EShooterSpin::NoFlip;
					LastPitchRotation = 0.f;
					LastYawRotation = 0.f;
					// LastVelocity = FVector::ZeroVector; TODO
					SphereLastVelocity = FVector::ZeroVector;
					SetActorRotation(FRotationMatrix::MakeFromZX(Hit.ImpactNormal, GetActorForwardVector()).Rotator());
				}
				else
				{
					MagnetizePressed(1.f);
					// SetFloorStatus(EShooterFloorStatus::NoFloorContact); TODO
					// LastVelocity += Hit.ImpactNormal * (LastVelocity.Size()/2.f); TODO
				}
			}
		}
		else
		{
			MagnetizePressed(1.f);
			// SetFloorStatus(EShooterFloorStatus::NoFloorContact); TODO
			// LastVelocity += Hit.ImpactNormal * (LastVelocity.Size()/2.f); TODO
		}
		bIsJumpingOffSphereLevel = false;
		JumpForce = FVector::ZeroVector;
		CurrentJumpVelocity = FVector::ZeroVector;
	}
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

FVector ABasePawnPlayer::GetHitTarget()
{
	if(Combat)
	{
		return Combat->HitTarget;
	}
	return FVector::ZeroVector;
	
}

void ABasePawnPlayer::PassDamageToHealth(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	Health->TakeDamage(DamagedActor, Damage, DamageType, InstigatedBy, DamageCauser);
}

EShooterFloorStatus ABasePawnPlayer::SetFloorStatus(EShooterFloorStatus InFloorStatus, FShooterStatus& StatusToReset)
{
	if(InFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		ZeroOutGravity(StatusToReset);
	}
	return InFloorStatus;
}

void ABasePawnPlayer::ZeroOutGravity(FShooterStatus& StatusToReset)
{
	StatusToReset.ClosestDistanceToFloor = FLT_MAX;
	StatusToReset.ClosestFloor = nullptr;
	const FHitResult NewHitResult;
	StatusToReset.FloorHitResult = NewHitResult;
	StatusToReset.CurrentGravity = FVector::ZeroVector;
}


void ABasePawnPlayer::ServerSendMove_Implementation(FShooterMove ClientMove)
{
	CSPStatus = StatusOnServer;
	
	CSPStatus.CurrentVelocity += Movement_Internal(ClientMove.MovementVector, ClientMove.ShooterLocation, ClientMove.ShooterRotationBeforeMovement, CSPStatus.CurrentVelocity, FixedTimeStep);
	Magnetize_Internal(ClientMove.bMagnetizedPressed, CSPStatus);
	CSPStatus.ShooterRotation = ClientMove.ShooterRotationAfterMovement;
	CSPStatus.SpringArmPitch = ClientMove.SpringArmPitch;
	CSPStatus.LastPitchRotation = ClientMove.LastPitchRotation;
	CSPStatus.LastYawRotation = ClientMove.LastYawRotation;
	CSPStatus.ShooterLocation = StatusOnServer.ShooterLocation + CSPStatus.CurrentVelocity;
	
	StatusOnServer = CSPStatus;
	StatusOnServer.LastMove = ClientMove;
	bSetStatusAfterUpdate = true;
}

void ABasePawnPlayer::OnRep_StatusOnServer()
{
	CSPStatus = StatusOnServer;
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
	if(!bIsInterpolatingClientStatus)
	{
		for(const FShooterMove MoveToPlay: UnacknowledgedMoves)
		{
			CSPStatus.CurrentVelocity += Movement_Internal(MoveToPlay.MovementVector, MoveToPlay.ShooterLocation, MoveToPlay.ShooterRotationBeforeMovement, CSPStatus.CurrentVelocity, FixedTimeStep);
			CSPStatus.ShooterLocation += CSPStatus.CurrentVelocity;
			DrawDebugPoint(GetWorld(), CSPStatus.ShooterLocation, 30.f, FColor::Blue);
		}
		CurrentCSPLocationDelta = (GetActorLocation() - CSPStatus.ShooterLocation).Size();
	}
	if(bIsInDebugMode)
	{
		DrawDebugPoint(GetWorld(), CSPStatus.ShooterLocation, 20.f, FColor::Green);
	}
}

void ABasePawnPlayer::InterpAutonomousCSPTransform(float DeltaTime)
{
	if(!HasAuthority() && IsLocallyControlled())
	{
		if(CurrentCSPLocationDelta > ServerClintDeltaTolerance)
		{
			const FVector CurrentVector = GetActorLocation();
			const FVector ToServerLocation = FMath::VInterpTo(CurrentVector,  CSPStatus.ShooterLocation, DeltaTime, ServerCorrectionSpeed);
			CurrentCSPLocationDelta = (GetActorLocation() - CSPStatus.ShooterLocation).Size();
			SetActorLocation(ToServerLocation);
			UnacknowledgedMoves.Empty();
			bIsInterpolatingClientStatus = true;
		}
		else
		{
			const FVector CurrentVector = GetActorLocation();
			const FVector ToServerLocation = FMath::VInterpTo(CurrentVector, CSPStatus.ShooterLocation, DeltaTime, IdleServerCorrectionSpeed);
			CurrentCSPLocationDelta = (GetActorLocation() - CSPStatus.ShooterLocation).Size();
			SetActorLocation(ToServerLocation);
			bIsInterpolatingClientStatus = false;
		}
	}
}

void ABasePawnPlayer::MoveClientProxies(float DeltaTime)
{
	if(!IsLocallyControlled())
	{
		if(bSetStatusAfterUpdate) //while the current location is far away, InterpTo
		{
			const FVector CurrentLocation = GetActorLocation();
			const FVector NewLocation = StatusOnServer.ShooterLocation;
			const FVector InterpLocation = FMath::VInterpTo(CurrentLocation, NewLocation, DeltaTime, ProxyCorrectionSpeed);
			SetActorLocation(InterpLocation);
			const FRotator CurrentRotation = GetActorRotation();
			const FRotator NewRotation = StatusOnServer.ShooterRotation;
			const FRotator InterpRotation = FMath::RInterpTo(CurrentRotation, NewRotation, DeltaTime, ProxyCorrectionSpeed);
			SetActorRotation(InterpRotation);
			bSetStatusAfterUpdate = false;
		}
		else if(!bSetStatusAfterUpdate)//else keep the actor going its last velocity extrapolate 
		{
			AddActorWorldOffset(StatusOnServer.CurrentVelocity);
			AddActorLocalRotation(FRotator(StatusOnServer.LastPitchRotation, StatusOnServer.LastYawRotation, 0.f));
		}
		// DrawDebugPoint(GetWorld(), StatusOnServer.ShooterLocation, 20.f, FColor::Blue);
	}
}

float ABasePawnPlayer::GetSpringArmPitch() const
{
	if(IsLocallyControlled())
	{
		return LocalStatus.SpringArmPitch; 
	}
	return StatusOnServer.SpringArmPitch;
}

bool ABasePawnPlayer::GetIsMagnetized() const
{
	if(IsLocallyControlled())
	{
		return LocalStatus.bMagnetized; 
	}
	return StatusOnServer.bMagnetized;
}

void ABasePawnPlayer::DebugMode() const
{
	if(bIsInDebugMode && IsLocallyControlled())
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (CurrentVelocity * 10.f), FColor::Green);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,0.f, FColor::Green, FString::Printf(TEXT("%s"), *GetName()));
			const FColor CSPDeltaColor = CurrentCSPLocationDelta > ServerClintDeltaTolerance ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, CSPDeltaColor, FString::Printf(TEXT("CurrentCSPLocationDelta: %f"), CurrentCSPLocationDelta));
			const FColor BoostCountColor = BoostCount == 0 ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, BoostCountColor, FString::Printf(TEXT("BoostCount: %i"), BoostCount));
			const FColor MagnetizeColor = bIsMagnetized ? FColor::Green : FColor::Red;
			const FString MagnetizeString = bIsMagnetized ? FString(TEXT("bIsMagnetized: True")) : FString(TEXT("bIsMagnetized: False"));
			GEngine->AddOnScreenDebugMessage(-1,0.f, MagnetizeColor, MagnetizeString);
			const FColor JumpBoolColor = bIsJumpingOffSphereLevel ? FColor::Green : FColor::Red;
			const FString JumpBoolString = bIsJumpingOffSphereLevel ? FString(TEXT("JumpingOffLevelSphere: True")) : FString(TEXT("JumpingOffLevelSphere: False"));
			GEngine->AddOnScreenDebugMessage(-1,0.f, JumpBoolColor, JumpBoolString);
			const FColor JumpVelocityColor = CurrentJumpVelocity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, JumpVelocityColor, FString::Printf(TEXT("CurrentJumpVelocity: %s"), *CurrentJumpVelocity.ToString()));
			const FColor VelocityColor = CurrentVelocity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, VelocityColor, FString::Printf(TEXT("CurrentVelocity: %s"), *CurrentVelocity.ToString()));
			const FColor SphereVelocity = SphereLastVelocity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, SphereVelocity, FString::Printf(TEXT("SphereLastVelocity: %s"), *SphereLastVelocity.ToString()));
			const FColor JumpColor = JumpForce.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, JumpColor, FString::Printf(TEXT("JumpForce: %s"), *JumpForce.ToString()));
			const FColor GravityColor = CurrentGravity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, GravityColor, FString::Printf(TEXT("CurrentGravity: %s"), *CurrentGravity.ToString()));
			const FColor FloorColor = ClosestFloor == nullptr ? FColor::Red : FColor::Green;
			if(ClosestFloor)
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorColor, FString::Printf(TEXT("ClosestFloor: %s"), *ClosestFloor->GetName()));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorColor, FString::Printf(TEXT("ClosestFloor: NoFloor")));
			}
			const FColor FloorHitResultColor = FloorHitResult.bBlockingHit == false ? FColor::Red : FColor::Green;
			if(ClosestFloor)
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorHitResultColor, FString::Printf(TEXT("FloorHitResultImpact: %s"), *FloorHitResult.ImpactPoint.ToString()));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorHitResultColor, FString::Printf(TEXT("FloorHitResultImpact: NoImpact")));
			}
		}
		
	}
}

void ABasePawnPlayer::SwitchDebugMode()
{
	bIsInDebugMode = !bIsInDebugMode;
}
