// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePawnPlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "VectorTypes.h"
#include "Chaos/SpatialAccelerationCollection.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
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
	FootSphere = CreateDefaultSubobject<USphereComponent>("FootCheck");
	FootSphere->SetupAttachment(Skeleton);
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
	if(Capsule && FootSphere && IsLocallyControlled())
	{
		Capsule->OnComponentHit.AddDynamic(this, &ABasePawnPlayer::OnFloorHit);
		FootSphere->OnComponentEndOverlap.AddDynamic(this, &ABasePawnPlayer::EndFloorCheck);
	}
	OnTakeAnyDamage.AddDynamic(this, &ABasePawnPlayer::PassDamageToHealth);
	if(IsLocallyControlled())
	{
		LocalStatus.SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
		LocalStatus.SpringArmYaw = SpringArm->GetRelativeRotation().Yaw;
		LocalStatus.ShooterLocation = GetActorLocation();
		LocalStatus.ShooterRotation = GetActorRotation();
		LocalStatus.BoostCount = MaxBoosts;
	}
	else
	{
		StatusOnServer.SpringArmPitch = SpringArm->GetRelativeRotation().Pitch;
		StatusOnServer.SpringArmYaw = SpringArm->GetRelativeRotation().Yaw;
		StatusOnServer.ShooterLocation = GetActorLocation();
		StatusOnServer.ShooterRotation = GetActorRotation();
		StatusOnServer.BoostCount = MaxBoosts;
	}
}

void ABasePawnPlayer::Tick(const float DeltaTime)
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

void ABasePawnPlayer::ShooterMovement(const float DeltaTime)
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
			if(GetWorld() && GetWorld()->GetGameState()) MoveToSend.GameTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
			
			LocalStatus.CurrentVelocity = Movement_Internal(MoveToSend.MovementVector, LocalStatus, DeltaTime);
			Magnetize_Internal(MoveToSend.bMagnetizedPressed, LocalStatus);
			Boost_Internal(MoveToSend.BoostDirection, MoveToSend.bBoost, false, LocalStatus);
			SetActorTransform(PerformGravity(LocalStatus, DeltaTime));
			AddActorWorldOffset(Jump_Internal(MoveToSend.bJumped, LocalStatus, DeltaTime));
		}
		SpringArm->SetRelativeRotation(PitchLook_Internal(LocalStatus, DeltaTime));
		AddActorLocalRotation(AddShooterSpin_Internal(LocalStatus, DeltaTime));
		AddActorLocalRotation(YawLook_Internal(LocalStatus, DeltaTime));
		AddActorWorldOffset(LocalStatus.CurrentVelocity);
		
		LocalStatus.ShooterLocation = GetActorLocation();
		LocalStatus.ShooterRotation = GetActorRotation();
		MoveToSend.LastPitchRotation = LocalStatus.LastPitchRotation;
		MoveToSend.LastYawRotation = LocalStatus.LastYawRotation;
		MoveToSend.ShooterRotationAfterMovement = LocalStatus.ShooterRotation;
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

FVector ABasePawnPlayer::Movement_Internal(const FVector ActionValue, FShooterStatus& OutStatus, const float DeltaTime) const
{
	const FVector InputMovementVector = TotalMovementInput(ActionValue, OutStatus, DeltaTime);
	return CalculateMovementVelocity(InputMovementVector,OutStatus, DeltaTime);
}

FVector ABasePawnPlayer::TotalMovementInput(const FVector ActionValue, const FShooterStatus InStatus, const float DeltaTime) const
{
	const FVector ForwardVector = InStatus.ShooterRotation.Quaternion().GetAxisX();
	const FVector RightVector = InStatus.ShooterRotation.Quaternion().GetAxisY();
	const FVector UpVector = InStatus.ShooterRotation.Quaternion().GetAxisZ();
	
	if(InStatus.ShooterFloorStatus != EShooterFloorStatus::NoFloorContact) //if contacted with a floor
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
		return (ForwardVector * ActionValue.X + RightVector * ActionValue.Y + UpVector * ActionValue.Z) * DeltaTime;
	}
	return FVector::ZeroVector;
}

FVector ABasePawnPlayer::CalculateMovementVelocity(const FVector InMovementInput, FShooterStatus& OutStatus, const float DeltaTime) const
{
	if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::BaseFloorContact && OutStatus.bMagnetized) //if walking on a flat floor and magnetized
	{
		if(InMovementInput.Size() == 0.f)
		{
			return FMath::VInterpTo(OutStatus.CurrentVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
		}
		return  FMath::VInterpTo(OutStatus.CurrentVelocity, InMovementInput, DeltaTime, AccelerationSpeed);
	}
	if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereFloorContact && OutStatus.bMagnetized) //if walking on a sphere and magnetized
	{
		if(InMovementInput.Size() == 0.f)
		{
			OutStatus.SphereLastVelocity = FMath::VInterpTo(OutStatus.SphereLastVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
			const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(OutStatus.SphereLastVelocity, OutStatus.ShooterRotation.Quaternion().GetAxisZ());
			const FVector SphereToActor = OutStatus.ShooterLocation - OutStatus.SphereLocation;
			const FVector NewPosition = SphereToActor.RotateAngleAxis(OutStatus.SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
			return NewPosition - SphereToActor;
		}
		const FVector AdjustedControlInputVector = InMovementInput * SphereFloorMovementPercent;
		OutStatus.SphereLastVelocity = FMath::VInterpTo(OutStatus.SphereLastVelocity, AdjustedControlInputVector, DeltaTime, AccelerationSpeed);
		const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(OutStatus.SphereLastVelocity, OutStatus.ShooterRotation.Quaternion().GetAxisZ());
		const FVector SphereToActor = OutStatus.ShooterLocation - OutStatus.SphereLocation;
		const FVector NewPosition = SphereToActor.RotateAngleAxis(OutStatus.SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
		return NewPosition - SphereToActor;
	}
	if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereLevelContact && OutStatus.bMagnetized) //if walking in a sphere and magnetized OR jumping on level sphere
	{
		if(InMovementInput.Size() == 0.f)
		{
			OutStatus.SphereLastVelocity = FMath::VInterpTo(OutStatus.SphereLastVelocity, FVector::ZeroVector, DeltaTime, StoppingSpeed);
			const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(OutStatus.SphereLastVelocity, OutStatus.ShooterRotation.Quaternion().GetAxisZ());
			const FVector SphereToActor = OutStatus.ShooterLocation - OutStatus.SphereLocation;
			const FVector NewPosition = SphereToActor.RotateAngleAxis(OutStatus.SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
			return NewPosition - SphereToActor;
		}
		const FVector AdjustedControlInputVector = -InMovementInput * LevelSphereMovementPercent;
		OutStatus.SphereLastVelocity = FMath::VInterpTo(OutStatus.SphereLastVelocity, AdjustedControlInputVector, DeltaTime, AccelerationSpeed);
		const FMatrix InputRotation = FRotationMatrix::MakeFromXZ(OutStatus.SphereLastVelocity, OutStatus.ShooterRotation.Quaternion().GetAxisZ());
		const FVector SphereToActor = OutStatus.ShooterLocation - OutStatus.SphereLocation;
		const FVector NewPosition = SphereToActor.RotateAngleAxis(OutStatus.SphereLastVelocity.Size(), InputRotation.GetUnitAxis(EAxis::Y));
		return NewPosition - SphereToActor;
	}
	return OutStatus.CurrentVelocity + InMovementInput * AirSpeed;
}

void ABasePawnPlayer::LookActivated(const FInputActionValue& ActionValue)
{
	PitchValue = ActionValue.Get<FVector2D>().Y;
	YawValue = ActionValue.Get<FVector2D>().X;
}

FRotator ABasePawnPlayer::PitchLook_Internal(FShooterStatus& OutStatus, float DeltaTime)
{
	OutStatus.SpringArmPitch = FMath::Clamp(OutStatus.SpringArmPitch + (PitchValue * DeltaTime * SpringArmPitchSpeed), SpringArmPitchMin, SpringArmPitchMax);
	if(OutStatus.SpringArmPitch > (SpringArmPitchMax - 1.5f) && OutStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		OutStatus.ShooterSpin = EShooterSpin::BackFlip;
	}
	else if(OutStatus.SpringArmPitch < (SpringArmPitchMin + 1.5f) && OutStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		OutStatus.ShooterSpin = EShooterSpin::FrontFlip;
	}
	else
	{
		OutStatus.ShooterSpin = EShooterSpin::NoFlip;
	}
	PitchValue = 0.f;
	return FRotator(OutStatus.SpringArmPitch, OutStatus.SpringArmYaw, 0.f);
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
				return FRotator(InStatus.LastPitchRotation, 0.f, 0.f);
			}
			NewPitchRotation = FMath::Clamp(InStatus.LastPitchRotation + PitchValue * AirPitchSpeed * DeltaTime, -MaxPitchSpeed, MaxPitchSpeed);
			return FRotator(InStatus.LastPitchRotation = NewPitchRotation, 0.f, 0.f);
		case EShooterSpin::FrontFlip:
			if(PitchValue >= 0.f)
			{
				return FRotator(InStatus.LastPitchRotation, 0.f, 0.f);
			}
			NewPitchRotation = FMath::Clamp(InStatus.LastPitchRotation - PitchValue * -AirPitchSpeed * DeltaTime, -MaxPitchSpeed, MaxPitchSpeed);
			return FRotator(InStatus.LastPitchRotation = NewPitchRotation, 0.f, 0.f);
		case EShooterSpin::NoFlip:
			return FRotator(InStatus.LastPitchRotation, 0.f, 0.f);
		default:
			return FRotator(InStatus.LastPitchRotation, 0.f, 0.f);
		}
	}
	return FRotator(LocalStatus.LastPitchRotation, 0.f, 0.f);
}

FRotator ABasePawnPlayer::YawLook_Internal(FShooterStatus& OutStatus, float DeltaTime)
{
	//We are contacted to a floor
	if(OutStatus.ShooterFloorStatus != EShooterFloorStatus::NoFloorContact)
	{
		const float FloorValue = YawValue;
		YawValue = 0.f;
		return FRotator(0.f, FloorValue, 0.f);
	}
	//We are not contacted to a floor
	if(YawValue == 0.f)
	{
		return FRotator(0.f, OutStatus.LastYawRotation, 0.f);
	}
	const float NewYawRotation = FMath::Clamp(OutStatus.LastYawRotation + YawValue * AirRotationSpeed * DeltaTime, -AirRotationMaxSpeed, AirRotationMaxSpeed);
	YawValue = 0.f;
	return FRotator(0.f, OutStatus.LastYawRotation = NewYawRotation, 0.f);
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

FVector ABasePawnPlayer::Jump_Internal(const bool bJumpWasPressed, FShooterStatus& OutStatus, const float DeltaTime)
{
	if(!FMath::IsNearlyZero(OutStatus.JumpForce.Size()) && OutStatus.bMagnetized)
	{
		OutStatus.JumpForce = FMath::VInterpTo(OutStatus.JumpForce, FVector::ZeroVector, DeltaTime, JumpDeceleration);
		OutStatus.CurrentJumpVelocity = OutStatus.ShooterLocation - OutStatus.LastJumpPosition;
		OutStatus.LastJumpPosition = OutStatus.ShooterLocation;
		return OutStatus.JumpForce;
	}
	if(bJumpWasPressed)
	{
		if(OutStatus.ShooterFloorStatus != EShooterFloorStatus::NoFloorContact && OutStatus.bMagnetized) //if we are in contact with a floor
		{
			if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereLevelContact)
			{
				OutStatus.bIsJumpingOffSphereLevel = true;
			}
			OutStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::NoFloorContact, OutStatus);
			OutStatus.JumpForce = OutStatus.ShooterRotation.Quaternion().GetAxisZ() * JumpVelocity + OutStatus.CurrentVelocity;
			OutStatus.CurrentVelocity = FVector::ZeroVector;
			OutStatus.LastJumpPosition = OutStatus.ShooterLocation;
			return OutStatus.JumpForce;
		}
	}
	return OutStatus.JumpForce = FVector::ZeroVector;
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
		if(OutStatus.CurrentJumpVelocity.Size() > 0)
		{
			OutStatus.CurrentVelocity = OutStatus.CurrentJumpVelocity;
			if(OutStatus.bIsJumpingOffSphereLevel)
			{
				OutStatus.SphereLastVelocity = OutStatus.CurrentJumpVelocity;
				OutStatus.bIsJumpingOffSphereLevel = false;
			}
			OutStatus.CurrentJumpVelocity = FVector::ZeroVector;
		}
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

void ABasePawnPlayer::Boost_Internal(FVector BoostVector, bool bBoostWasPressed, bool bIsPredictiveState, FShooterStatus& OutStatus)
{
	if(bBoostWasPressed)
	{
		if(OutStatus.BoostCount > 0)
		{
			FTransform InActorTransform;
			InActorTransform.SetLocation(OutStatus.ShooterLocation);
			InActorTransform.SetRotation(OutStatus.ShooterRotation.Quaternion());
			OutStatus.BoostCount --;
			if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
			{
				const FVector WorldBoostVector = InActorTransform.TransformVectorNoScale(BoostVector);
				OutStatus.CurrentVelocity = WorldBoostVector * NonContactedBoostSpeed + OutStatus.CurrentVelocity /= BoostLastVelocityReduction;
				if(bIsPredictiveState)
				{
					GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRechargePredictive, BoostRechargeRate);
				}
				else
				{
					GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRechargeConfirmed, BoostRechargeRate);
				}
			}
			else
			{
				ContactedBoostForce(BoostVector, OutStatus);
				if(bIsPredictiveState)
				{
					GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRechargePredictive, BoostRechargeRate);
				}
				else
				{
					GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRechargeConfirmed, BoostRechargeRate);
				}
			}
		}
	}
}

void ABasePawnPlayer::ContactedBoostForce(const FVector BoostVector, FShooterStatus& OutStatus) const
{
	FTransform InActorTransform;
	InActorTransform.SetLocation(OutStatus.ShooterLocation);
	InActorTransform.SetRotation(OutStatus.ShooterRotation.Quaternion());
	const FVector WorldBoostVector = ContactedBoostSpeed * InActorTransform.TransformVectorNoScale(BoostVector);
	if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::BaseFloorContact)
	{
		OutStatus.CurrentVelocity = WorldBoostVector;
	}
	if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereFloorContact)
	{
		OutStatus.SphereLastVelocity = WorldBoostVector * SphereFloorMovementPercent;
	}
	if(OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereLevelContact)
	{
		OutStatus.SphereLastVelocity = -WorldBoostVector * LevelSphereMovementPercent;
	}
}

void ABasePawnPlayer::BoostRechargeConfirmed()
{
	LocalStatus.BoostCount++;
	if(LocalStatus.BoostCount < MaxBoosts)
	{
		GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRechargeConfirmed, BoostRechargeRate);
	}
}

void ABasePawnPlayer::BoostRechargePredictive()
{
	CSPStatus.BoostCount++;
	if(CSPStatus.BoostCount < MaxBoosts)
	{
		GetWorldTimerManager().SetTimer(BoostRechargeTimerHandle, this, &ABasePawnPlayer::BoostRechargePredictive, BoostRechargeRate);
	}
}

FTransform ABasePawnPlayer::PerformGravity(FShooterStatus& OutStatus, const float DeltaTime) const
{
	FTransform NewActorTransform;
	NewActorTransform.SetLocation(OutStatus.ShooterLocation);
	NewActorTransform.SetRotation(OutStatus.ShooterRotation.Quaternion());
	if(OutStatus.bMagnetized && OutStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact)
	{
		if(OutStatus.bIsJumpingOffSphereLevel)
		{
			FindClosestFloor(NewActorTransform, OutStatus);
			if(OutStatus.ClosestFloor != nullptr)
			{
				OutStatus.SphereLocation = OutStatus.ClosestFloor->GetActorLocation();
				OutStatus.CurrentGravity = NewActorTransform.GetLocation() - OutStatus.SphereLocation;
				NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), OutStatus, DeltaTime).Quaternion());
				OutStatus.LastPitchRotation = 0.f;
				NewActorTransform.SetLocation(GravityForce(NewActorTransform.GetLocation(), OutStatus, DeltaTime));
			}
		}
		else
		{
			FindClosestFloor(NewActorTransform, OutStatus);
			if(OutStatus.ClosestFloor != nullptr)
			{
				NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), OutStatus, DeltaTime).Quaternion());
				OutStatus.LastPitchRotation = 0.f;
				NewActorTransform.SetLocation(GravityForce(NewActorTransform.GetLocation(), OutStatus, DeltaTime));
			}
		}
		return NewActorTransform;
	}
	if(OutStatus.bMagnetized && OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereFloorContact)
	{
		if(OutStatus.ClosestFloor != nullptr)
		{
			OutStatus.SphereLocation = OutStatus.ClosestFloor->GetActorLocation();
		}
		OutStatus.CurrentGravity = OutStatus.SphereLocation - NewActorTransform.GetLocation();
		NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), OutStatus, DeltaTime).Quaternion());
		OutStatus.LastPitchRotation = 0.f;
		return NewActorTransform;
	}
	if(OutStatus.bMagnetized && OutStatus.ShooterFloorStatus == EShooterFloorStatus::SphereLevelContact)
	{
		if(OutStatus.ClosestFloor != nullptr)
		{
			OutStatus.SphereLocation = OutStatus.ClosestFloor->GetActorLocation();
		}
		if(OutStatus.bIsJumpingOffSphereLevel)
		{
			NewActorTransform.SetLocation(GravityForce(NewActorTransform.GetLocation(), OutStatus, DeltaTime));
		}
		OutStatus.CurrentGravity = NewActorTransform.GetLocation() - OutStatus.SphereLocation;
		NewActorTransform.SetRotation(OrientToGravity(NewActorTransform.Rotator(), OutStatus, DeltaTime).Quaternion());
		OutStatus.LastPitchRotation = 0.f;
		return NewActorTransform;
	}
	return NewActorTransform;
}

void ABasePawnPlayer::FindClosestFloor(FTransform ActorTransform, FShooterStatus& OutStatus) const
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
				World->SweepSingleByChannel(OutStatus.FloorHitResult, ActorTransform.GetLocation(), Floor.GetActor()->GetActorLocation(), FQuat::Identity, ECC_GameTraceChannel1, TraceShape, QueryParams, ResponseParams);
				if(OutStatus.FloorHitResult.bBlockingHit && (OutStatus.FloorHitResult.ImpactPoint - ActorTransform.GetLocation()).Size() < OutStatus.ClosestDistanceToFloor)
				{
					if(bIsInDebugMode)
					{
						DrawDebugPoint(World, OutStatus.FloorHitResult.ImpactPoint, 50.f, FColor::Red);
					}
					OutStatus.ClosestDistanceToFloor = (OutStatus.FloorHitResult.ImpactPoint - ActorTransform.GetLocation()).Size();
					OutStatus.ClosestFloor = Floor.GetActor();
					OutStatus.CurrentGravity = OutStatus.FloorHitResult.ImpactPoint - ActorTransform.GetLocation();
				}
			}
		}
	}
}

FRotator ABasePawnPlayer::OrientToGravity(const FRotator InActorRotation, const FShooterStatus InStatus, const float DeltaTime) const
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
	FeetToGravity = FRotationMatrix::MakeFromZX(-InStatus.CurrentGravity, InActorRotation.Quaternion().GetAxisX());
	FQuat NewRotation;
	if(InStatus.ShooterFloorStatus == EShooterFloorStatus::SphereFloorContact || InStatus.ShooterFloorStatus == EShooterFloorStatus::SphereLevelContact)
	{
		NewRotation = FQuat::Slerp(InActorRotation.Quaternion(),FeetToGravity.ToQuat(), 1.f);
	}
	else
	{
		NewRotation = FQuat::Slerp(InActorRotation.Quaternion(),FeetToGravity.ToQuat(), DeltaTime * (SlerpSpeed/InStatus.ClosestDistanceToFloor));
	}
	return NewRotation.Rotator();
}

FVector ABasePawnPlayer::GravityForce(const FVector InActorLocation, FShooterStatus& OutStatus, const float DeltaTime) const
{
	OutStatus.ClosestDistanceToFloor = OutStatus.FloorHitResult.Distance + SphereTraceRadius;
	const float DistancePct = FMath::Abs(150 - 100 * (OutStatus.ClosestDistanceToFloor/GravityDistanceRadius));
	FVector NewVector;
	if(OutStatus.FloorHitResult.bBlockingHit && DistancePct == 100.f)
	{
		NewVector = FMath::VInterpConstantTo(InActorLocation, FVector(OutStatus.FloorHitResult.ImpactPoint), DeltaTime, InRangeGravityStrength);
	}
	else if(OutStatus.FloorHitResult.bBlockingHit)
	{
		NewVector = FMath::VInterpConstantTo(InActorLocation, FVector(OutStatus.FloorHitResult.ImpactPoint), DeltaTime, FMath::Pow(OutRangeGravityStrength * DistancePct, GravityForceCurve));
	}
	OutStatus.CurrentVelocity = FMath::VInterpTo(OutStatus.CurrentVelocity, FVector::ZeroVector, DeltaTime, GravityVelocityReduction);
	OutStatus.SphereLastVelocity = FVector::ZeroVector;
	return NewVector;
}

void ABasePawnPlayer::OnFloorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if(LocalStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact && LocalStatus.bMagnetized)
	{
		if(FVector::DotProduct(GetActorUpVector(), Hit.ImpactNormal) > 0.9f)
		{
			if(const ASphereFloorBase* SphereFloor = Cast<ASphereFloorBase>(OtherActor))
			{
				LocalStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::SphereFloorContact, LocalStatus);
				LocalStatus.ShooterSpin = EShooterSpin::NoFlip;
				LocalStatus.LastPitchRotation = 0.f;
				LocalStatus.LastYawRotation = 0.f;
				LocalStatus.CurrentVelocity = FVector::ZeroVector;
				LocalStatus.SphereLastVelocity = FVector::ZeroVector;
				SetActorRotation(FRotationMatrix::MakeFromZX(Hit.ImpactNormal, GetActorForwardVector()).Rotator());
			}
			else if(const AGravitySphere* LevelSphere = Cast<AGravitySphere>(OtherActor))
			{
				LocalStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::SphereLevelContact, LocalStatus);
				LocalStatus.ShooterSpin = EShooterSpin::NoFlip;
				LocalStatus.LastPitchRotation = 0.f;
				LocalStatus.LastYawRotation = 0.f;
				LocalStatus.CurrentVelocity = FVector::ZeroVector;
				LocalStatus.SphereLastVelocity = FVector::ZeroVector;
				SetActorRotation(FRotationMatrix::MakeFromZX(Hit.ImpactNormal, GetActorForwardVector()).Rotator());
			}
			else if(const AFloorBase* Floor = Cast<AFloorBase>(OtherActor))
			{
				const float DotProductResult = FVector::DotProduct(Floor->GetActorUpVector(), Hit.ImpactNormal);
				constexpr float Epsilon = 0.001f;
				if(FMath::IsNearlyEqual(DotProductResult, 1.f, Epsilon) || FMath::IsNearlyEqual(DotProductResult, -1.f, Epsilon))
				{
					LocalStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::BaseFloorContact, LocalStatus);
					LocalStatus.ShooterSpin = EShooterSpin::NoFlip;
					LocalStatus.LastPitchRotation = 0.f;
					LocalStatus.LastYawRotation = 0.f;
					LocalStatus.CurrentVelocity = FVector::ZeroVector;
					LocalStatus.SphereLastVelocity = FVector::ZeroVector;
					SetActorRotation(FRotationMatrix::MakeFromZX(Hit.ImpactNormal, GetActorForwardVector()).Rotator());
				}
				else
				{
					MagnetizePressed(1.f);
					LocalStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::NoFloorContact, LocalStatus);
					LocalStatus.CurrentVelocity += Hit.ImpactNormal * (LocalStatus.CurrentVelocity.Size()/2.f);
				}
			}
		}
		else
		{
			MagnetizePressed(1.f);
			LocalStatus.ShooterFloorStatus = SetFloorStatus(EShooterFloorStatus::NoFloorContact, LocalStatus);
			LocalStatus.CurrentVelocity += Hit.ImpactNormal * (LocalStatus.CurrentVelocity.Size()/2.f);
		}
		LocalStatus.bIsJumpingOffSphereLevel = false;
		LocalStatus.CurrentJumpVelocity = FVector::ZeroVector;
	}
}

void ABasePawnPlayer::EndFloorCheck(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("END: %f"), GetWorld()->TimeSeconds);
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

EShooterFloorStatus ABasePawnPlayer::SetFloorStatus(const EShooterFloorStatus StatusToChangeTo, FShooterStatus& StatusToReset)
{
	if(StatusToChangeTo == EShooterFloorStatus::NoFloorContact)
	{
		ZeroOutGravity(StatusToReset);
	}
	return StatusToChangeTo;
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
	
	CSPStatus.CurrentVelocity = Movement_Internal(ClientMove.MovementVector, CSPStatus, FixedTimeStep);
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
			CSPStatus.CurrentVelocity = Movement_Internal(MoveToPlay.MovementVector, CSPStatus, FixedTimeStep);
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
		DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + (LocalStatus.CurrentVelocity * 10.f), FColor::Green);
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,0.f, FColor::Green, FString::Printf(TEXT("%s"), *GetName()));
			const FColor CSPDeltaColor = CurrentCSPLocationDelta > ServerClintDeltaTolerance ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, CSPDeltaColor, FString::Printf(TEXT("CurrentCSPLocationDelta: %f"), CurrentCSPLocationDelta));
			const FColor BoostCountColor = LocalStatus.BoostCount == 0 ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, BoostCountColor, FString::Printf(TEXT("BoostCount: %i"), LocalStatus.BoostCount));
			const FColor MagnetizeColor = LocalStatus.bMagnetized ? FColor::Green : FColor::Red;
			const FString MagnetizeString = LocalStatus.bMagnetized ? FString(TEXT("bIsMagnetized: True")) : FString(TEXT("bIsMagnetized: False"));
			GEngine->AddOnScreenDebugMessage(-1,0.f, MagnetizeColor, MagnetizeString);
			const FColor JumpBoolColor = LocalStatus.bIsJumpingOffSphereLevel ? FColor::Green : FColor::Red;
			const FString JumpBoolString = LocalStatus.bIsJumpingOffSphereLevel ? FString(TEXT("JumpingOffLevelSphere: True")) : FString(TEXT("JumpingOffLevelSphere: False"));
			GEngine->AddOnScreenDebugMessage(-1,0.f, JumpBoolColor, JumpBoolString);
			const FColor JumpVelocityColor = LocalStatus.CurrentJumpVelocity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, JumpVelocityColor, FString::Printf(TEXT("CurrentJumpVelocity: %s"), *LocalStatus.CurrentJumpVelocity.ToString()));
			const FColor VelocityColor = LocalStatus.CurrentVelocity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, VelocityColor, FString::Printf(TEXT("CurrentVelocity: %f"), LocalStatus.CurrentVelocity.Size()));
			const FColor PitchColor = PitchValue == 0.f ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, PitchColor, FString::Printf(TEXT("PitchValue: %f"), PitchValue));
			const FColor LastPitchColor = LocalStatus.LastPitchRotation == 0.f ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, LastPitchColor, FString::Printf(TEXT("LastPitchRotation: %f"),  LocalStatus.LastPitchRotation));
			const FColor YawColor = PitchValue == 0.f ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, YawColor, FString::Printf(TEXT("YawValue: %f"), YawValue));
			const FColor LastYawColor = LocalStatus.LastYawRotation == 0.f ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, LastYawColor, FString::Printf(TEXT("LastYawRotation: %f"),  LocalStatus.LastYawRotation));
			const FColor SphereVelocity = LocalStatus.SphereLastVelocity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, SphereVelocity, FString::Printf(TEXT("SphereLastVelocity: %s"), *LocalStatus.SphereLastVelocity.ToString()));
			const FColor JumpColor = LocalStatus.JumpForce.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, JumpColor, FString::Printf(TEXT("JumpForce: %s"), *LocalStatus.JumpForce.ToString()));
			const FColor GravityColor = LocalStatus.CurrentGravity.IsZero() ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(-1,0.f, GravityColor, FString::Printf(TEXT("CurrentGravity: %s"), *LocalStatus.CurrentGravity.ToString()));
			const FColor FloorColor = LocalStatus.ClosestFloor == nullptr ? FColor::Red : FColor::Green;
			if(LocalStatus.ClosestFloor)
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorColor, FString::Printf(TEXT("ClosestFloor: %s"), *LocalStatus.ClosestFloor->GetName()));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorColor, FString::Printf(TEXT("ClosestFloor: NoFloor")));
			}
			const FColor FloorHitResultColor = LocalStatus.FloorHitResult.bBlockingHit == false ? FColor::Red : FColor::Green;
			if(LocalStatus.ClosestFloor)
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorHitResultColor, FString::Printf(TEXT("FloorHitResultImpact: %s"), *LocalStatus.FloorHitResult.ImpactPoint.ToString()));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1,0.f, FloorHitResultColor, FString::Printf(TEXT("FloorHitResultImpact: NoImpact")));
			}
			const FColor FloorStatusColor = LocalStatus.ShooterFloorStatus == EShooterFloorStatus::NoFloorContact ? FColor::Red : FColor::Green;
			FString FloorStatusString;
			switch (LocalStatus.ShooterFloorStatus)
			{
			case EShooterFloorStatus::NoFloorContact:
					FloorStatusString = FString(TEXT("FloorStatus: NoFloor"));
					break;
				case EShooterFloorStatus::BaseFloorContact:
					FloorStatusString = FString(TEXT("FloorStatus: BaseFloor"));
					break;
				case EShooterFloorStatus::SphereFloorContact:
					FloorStatusString = FString(TEXT("FloorStatus: SphereFloor"));
					break;
				case EShooterFloorStatus::SphereLevelContact:
					FloorStatusString = FString(TEXT("FloorStatus: SphereLevel"));
					break;
				default:
					FloorStatusString = FString(TEXT("FloorStatus: NoFloor"));
					break;
			}
			GEngine->AddOnScreenDebugMessage(-1,0.f, FloorStatusColor, FloorStatusString);
		}
	}
}

void ABasePawnPlayer::SwitchDebugMode()
{
	bIsInDebugMode = !bIsInDebugMode;
}
