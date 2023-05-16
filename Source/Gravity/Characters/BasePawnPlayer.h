// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gravity/Components/ShooterCombatComponent.h"
#include "Gravity/GravityTypes/ShooterFloorStatus.h"
#include "BasePawnPlayer.generated.h"

class UShooterHealthComponent;
class AWeaponBase;
class UBoxComponent;
class AFloorBase;
struct FHUDPackage;
class ASphereFloorBase;
class AGravitySphere;
class UCameraComponent;
class USpringArmComponent;
class UCapsuleComponent;
class UInputAction;
class UInputMappingContext;
class UShooterCombatComponent;

USTRUCT()
struct FShooterMove
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform MoveTransform = FTransform::Identity;
	UPROPERTY()
	FVector MovementVector = FVector::ZeroVector;
	UPROPERTY()
	float PitchRotation = 0.f;
	UPROPERTY()
	float AirSpin =  0.f;
	UPROPERTY()
	float YawRotation =  0.f;
	UPROPERTY()
	bool bJumped = false;
	UPROPERTY()
	bool bMagnetized = false;
	UPROPERTY()
	bool bBoost = false;
	UPROPERTY()
	FVector BoostDirection = FVector::ZeroVector;
	UPROPERTY()
	float GameTime;
	
};

USTRUCT()
struct FShooterStatus
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform ShooterTransform;
	UPROPERTY()
	bool bMagnetized;
	UPROPERTY()
	FShooterMove LastMove;
	UPROPERTY()
	FVector Velocity;
	UPROPERTY()
	FVector Torque;
	
};

UENUM()
enum class EShooterSpin : uint8
{
	FrontFlip UMETA(DisplayName = "Spin Forwards"),
	BackFlip UMETA(DisplayName = "Spin Backwards"),
	NoFlip UMETA(DisplayName = "Don't Spin"),
};

UCLASS()
class GRAVITY_API ABasePawnPlayer : public APawn
{
	GENERATED_BODY()

public:
	ABasePawnPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	EShooterFloorStatus FloorStatus = EShooterFloorStatus::NoFloorContact;
	
protected:
	virtual void BeginPlay() override;
	
	//Hit Boxes
	UPROPERTY(EditAnywhere)
	UBoxComponent* Head;
	UPROPERTY(EditAnywhere)
	UBoxComponent* Spine2;
	UPROPERTY(EditAnywhere)
	UBoxComponent* Hips;
	UPROPERTY(EditAnywhere)
	UBoxComponent* RightUpLeg;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftUpLeg;
	UPROPERTY(EditAnywhere)
	UBoxComponent* RightLeg;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftLeg;
	UPROPERTY(EditAnywhere)
	UBoxComponent* RightFoot;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftFoot;
	UPROPERTY(EditAnywhere)
	UBoxComponent* RightArm;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftArm;
	UPROPERTY(EditAnywhere)
	UBoxComponent* RightForeArm;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftForeArm;
	UPROPERTY(EditAnywhere)
	UBoxComponent* RightHand;
	UPROPERTY(EditAnywhere)
	UBoxComponent* LeftHand;
	
	//Components
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* Capsule;
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* Skeleton;
	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;
	UPROPERTY(EditAnywhere)
	UShooterCombatComponent* Combat;
	UPROPERTY(EditAnywhere)
	UShooterHealthComponent* Health;

	//Inputs Bindings
	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere)
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere)
	UInputAction* MagnetizeAction;
	UPROPERTY(EditAnywhere)
	UInputAction* BoostDirectionAction;
	UPROPERTY(EditAnywhere)
	UInputAction* BoostAction;
	UPROPERTY(EditAnywhere)
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere)
	UInputAction* FireAction;
	UPROPERTY(EditAnywhere)
	UInputMappingContext* CharacterMovementMapping;
	UPROPERTY(EditAnywhere)
	UInputMappingContext* CharacterCombatMapping;
	
	
	void Crouch(const FInputActionValue& ActionValue);
	
	void MagnetizePressed(const FInputActionValue& ActionValue);
	bool bMagnetizedPressed = false;
	void Magnetize_Internal(bool bMagnetizedWasPressed);
	UPROPERTY(Replicated)
	bool bIsMagnetized = false;


	void BoostPressed(const FInputActionValue& ActionValue);
	bool bBoostPressed = false;
	void Boost_Internal(FVector ActionValue, bool bBoostWasPressed);
	void BoostForce(const FVector ActionValue);
	void ContactedFloorMagnetizeDelay();
	FTimerHandle MagnetizeDelayForBoost;
	UPROPERTY(EditAnywhere, Category=Movement)
	float MagnetizeDelay = 1.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostCurrentVelocityReduction = 2.5f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostSpeed = 1000.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostRechargeRate = 5.f;
	bool bCanBoost = true;
	int8 BoostCount = 0;
	UPROPERTY(EditAnywhere, Category = Movement)
	int8 MaxBoosts = 2;
	void BoostCountConsumer();
	FTimerHandle BoostTimerHandle;

	UPROPERTY(EditAnywhere, Category=Movement)
	float FloorFriction = 4.f;
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirFriction = 0.01f;

	void Equip(const FInputActionValue& ActionValue);
	void FirePressed(const FInputActionValue& ActionValue);
	
	UFUNCTION()
	void OnFloorHit(UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float KnockBackImpulse = 1.75f;

private:
	//Input Functions
	const float FixedTimeStep = 1.f/60.f;
	float AccumulatedDeltaTime = 0.f;
	void ShooterMovement(float DeltaTime);
	void Magnetized(FShooterMove& OutMove);
	void Boost(FShooterMove& OutMove);
	FVector BoostDirection;
	/**
	 * @end 
	 */


	//everything involved with pressing forward, left, right, backward
	void MovePressed(const FInputActionValue& ActionValue);
	
	void BuildMovement(FShooterMove& OutMove);
	FVector MoveVector;
	
	FVector Movement_Internal(const FVector ActionValue, FTransform ActorTransform, float DeltaTime);
	
	void TotalMovementInput(const FVector ActionValue, FTransform ActorTransform, float DeltaTime);
	
	FVector CalculateMovementVelocity(FTransform ActorTransform, float DeltaTime);
	UPROPERTY(EditAnywhere, Category=Movement)
	float SphereFloorMovementPercent = 0.05f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float LevelSphereMovementPercent = 0.05f;
	FVector LastVelocity = FVector::ZeroVector;
	FVector SphereLastVelocity = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundForwardSpeed = 1000.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundBackwardSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundLateralSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundForwardLateralSpeed = 750.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float StoppingSpeed = 3.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float AccelerationSpeed = 4.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirSpeed = 2.5f;
	/**
	 * @end 
	 */


	//everything involved with mouse look rotation
	void LookActivated(const FInputActionValue& ActionValue);
	
	void BuildLook(FShooterMove& OutMove);
	float PitchValue = 0.f;
	float YawValue = 0.f;
	UPROPERTY(Replicated)
	float SpringArmClientPitch;
	
	FRotator PitchLook_Internal(float ActionValueY, float DeltaTime);
	float SpringArmPitch;
	float SpringArmYaw;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float SpringArmPitchSpeed = 10.f;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float SpringArmPitchMax = 70.f;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float SpringArmPitchMin = -75.f;
	EShooterSpin ShooterSpin = EShooterSpin::NoFlip;
	
	FRotator AddShooterSpin_Internal(float ActionValueY, float DeltaTime);
	float LastPitchRotation = 0.f;
	UPROPERTY(EditAnywhere, Category=MouseMovement)
	float AirPitchSpeed = 2.f;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float MaxPitchSpeed = 5.f;
	
	FRotator YawLook_Internal(float ActionValueX, float DeltaTime);
	float LastYawRotation = 0.f;
	UPROPERTY(EditAnywhere, Category=MouseMovement)
	float AirRotationSpeed = 0.25f;
	UPROPERTY(EditAnywhere, Category=MouseMovement)
	float AirRotationMaxSpeed = 2.f;
	/**
	* @end 
	*/



	//everything involved with gravity
	FTransform PerformGravity(FTransform InActorTransform, float DeltaTime);
	FVector SphereLocation = FVector::ZeroVector;
	float ClosestDistanceToFloor = FLT_MAX;
	UPROPERTY()
	AActor* ClosestFloor = nullptr;
	FHitResult FloorHitResult;
	FVector CurrentGravity = FVector::ZeroVector;
	
	AActor* FindClosestFloor(FTransform ActorTransform, FVector& OutGravityDirection);
	UPROPERTY(EditAnywhere, Category = Gravity)
	float SphereTraceRadius = 750.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float GravityDistanceRadius = 2500.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float ImpactEdgeAdjustment = 5.f;
	FVector OriginalActorLocation = FVector::ZeroVector;
	
	FRotator OrientToGravity(FRotator InActorRotator, float DeltaTime);
	UPROPERTY(EditAnywhere, Category = Gravity)
	float SlerpSpeed = 1000.f;
	
	FVector GravityForce(FVector InActorLocation, float DeltaTime);
	UPROPERTY(EditAnywhere, Category = Gravity)
	float OutRangeGravityStrength = 0.3f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float InRangeGravityStrength = 500.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float GravityForceCurve = 2.f;
	/**
	 * @end 
	 */

	//everything to do with jumping
	void JumpPressed(const FInputActionValue& ActionValue);
	
	void BuildJump(FShooterMove& OutMove);
	bool bJumpPressed = false;
	
	FVector Jump_Internal(bool bJumpWasPressed, FTransform ActorTransform, float DeltaTime);
	FVector CurrentJumpVelocity = FVector::ZeroVector;
	FVector LastJumpPosition = FVector::ZeroVector;
	FVector JumpForce = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpVelocity = 10.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpDeceleration = 10.f;
	/**
	 * @end
	 */

	
	UPROPERTY(EditAnywhere, Category = Movement)
	float RotationSpeedDampener = 100.f;
	UFUNCTION()
	void PassDamageToHealth(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION(Server, Reliable)
	void SendServerMove(FShooterMove ClientMove);

	TArray<FShooterMove> UnacknowledgedMoves;
	UPROPERTY(ReplicatedUsing = OnRep_StatusOnServer)
	FShooterStatus StatusOnServer;


	void ZeroOutGravity();
	UFUNCTION()
	void OnRep_StatusOnServer();
	void ClearAcknowledgedMoves();
	void PlayUnacknowledgedMoves();
	
	
public:
	FORCEINLINE EShooterFloorStatus GetFloorStatus() const {return FloorStatus;}
	void SetFloorStatus(EShooterFloorStatus InFloorStatus);
	FORCEINLINE float GetSpringArmPitch() const { return SpringArmPitch; }
	FORCEINLINE bool GetIsMagnetized() const {return bIsMagnetized;}
	FORCEINLINE void SetIsMagnetized(bool bMagnetization) { bIsMagnetized = bMagnetization; }
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Skeleton; }
	FVector GetHitTarget();
	FORCEINLINE UShooterCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE UShooterHealthComponent* GetHealthComponent() const { return Health; }
};
