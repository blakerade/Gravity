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

UENUM()
enum class EShooterSpin : uint8
{
	FrontFlip UMETA(DisplayName = "Spin Forwards"),
	BackFlip UMETA(DisplayName = "Spin Backwards"),
	NoFlip UMETA(DisplayName = "Don't Spin"),
};

USTRUCT()
struct FShooterMove
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector_NetQuantize MovementVector = FVector::ZeroVector;
	UPROPERTY()
	FRotator ShooterRotationAfterMovement;
	UPROPERTY()
	float LastPitchRotation;
	UPROPERTY()
	float LastYawRotation;
	UPROPERTY()
	float SpringArmPitch;
	UPROPERTY()
	bool bJumped = false;
	UPROPERTY()
	bool bMagnetizedPressed = false;
	UPROPERTY()
	bool bBoost = false;
	UPROPERTY()
	FVector_NetQuantize BoostDirection = FVector::ZeroVector;
	UPROPERTY()
	float GameTime;
};

USTRUCT()
struct FShooterStatus
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize ShooterLocation;
	UPROPERTY()
	FRotator ShooterRotation;
	UPROPERTY()
	float SpringArmPitch;
	UPROPERTY()
	float SpringArmYaw;
	UPROPERTY()
	float LastPitchRotation = 0.f;
	UPROPERTY()
	float LastYawRotation = 0.f;
	UPROPERTY()
	bool bMagnetized = false;
	UPROPERTY()
	int8 BoostCount;
	UPROPERTY()
	FVector_NetQuantize CurrentVelocity = FVector::ZeroVector;
	UPROPERTY()
	FVector_NetQuantize CurrentJumpVelocity = FVector::ZeroVector;
	UPROPERTY()
	FVector_NetQuantize JumpForce;
	UPROPERTY()
	FVector_NetQuantize LastJumpPosition;
	UPROPERTY()
	FVector_NetQuantize SphereLastVelocity = FVector::ZeroVector;
	UPROPERTY()
    FShooterMove LastMove;
	UPROPERTY()
	EShooterFloorStatus ShooterFloorStatus;
	UPROPERTY()
	EShooterSpin ShooterSpin = EShooterSpin::NoFlip;
	UPROPERTY()
	float ClosestDistanceToFloor = FLT_MAX;
	UPROPERTY()
	AActor* ClosestFloor = nullptr;
	UPROPERTY()
	FHitResult FloorHitResult;
	UPROPERTY()
	FVector_NetQuantize CurrentGravity;
	UPROPERTY()
	bool bIsJumpingOffSphereLevel = false;
	UPROPERTY()
	FVector_NetQuantize SphereLocation;
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

	void DebugMode() const;
	UFUNCTION(Exec)
	void SwitchDebugMode();
	bool bIsInDebugMode = false
	;
	
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
	//everything involved with network smoothing
	void InterpAutonomousCSPTransform(float DeltaTime);
	bool bIsInterpolatingClientStatus = false;
	bool bIsExtrapolating = false;
	
	float CurrentCSPLocationDelta = 0.f;
	UPROPERTY(EditAnywhere, Category=Network)
	float ServerClintDeltaTolerance = 200.f;
	UPROPERTY(EditAnywhere, Category=Network)
	float ServerCorrectionSpeed = 3.f;
	UPROPERTY(EditAnywhere, Category=Network)
	float IdleServerCorrectionSpeed = 0.15f;

	void MoveClientProxies(float DeltaTime);
	UPROPERTY(EditAnywhere, Category=Network)
	float ProxyCorrectionSpeed = 4.f;

	UFUNCTION(Server, Unreliable)
	void ServerSendMove(FShooterMove ClientMove);
	bool bSetStatusAfterUpdate = false;
	
	TArray<FShooterMove> UnacknowledgedMoves;
	UPROPERTY(ReplicatedUsing = OnRep_StatusOnServer)
	FShooterStatus StatusOnServer;
	FShooterStatus LocalStatus;
	FShooterStatus CSPStatus;
	
	UFUNCTION()
	void OnRep_StatusOnServer();
	
	void ClearAcknowledgedMoves();
	
	void PlayUnacknowledgedMoves();
	
	/**
	 * @end 
	 */
	
	//Input Functions
	const float FixedTimeStep = 1.f/60.f;
	float AccumulatedDeltaTime = 0.f;
	void ShooterMovement(float DeltaTime);
	/**
	 * @end 
	 */


	//everything involved with pressing forward, left, right, backward
	void MovePressed(const FInputActionValue& ActionValue);
	
	void BuildMovement(FShooterMove& OutMove);
	FVector MoveVector;
	
	FVector Movement_Internal(const FVector ActionValue, FShooterStatus& OutStatus, float DeltaTime) const;
	
	FVector TotalMovementInput(const FVector ActionValue, FShooterStatus InStatus, float DeltaTime) const;
	
	FVector CalculateMovementVelocity(FVector InMovementInput, FShooterStatus& OutStatus, float DeltaTime) const;
	UPROPERTY(EditAnywhere, Category=Movement)
	float SphereFloorMovementPercent = 0.025f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float LevelSphereMovementPercent = 0.0075f;
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
	float PitchValue = 0.f;
	float YawValue = 0.f;
	
	UPROPERTY(Replicated)
	float SpringArmClientPitch;
	
	FRotator PitchLook_Internal(FShooterStatus& OutStatus, float DeltaTime);
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float SpringArmPitchSpeed = 10.f;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float SpringArmPitchMax = 70.f;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float SpringArmPitchMin = -75.f;
	UPROPERTY()
	EShooterSpin ShooterSpin = EShooterSpin::NoFlip;
	
	FRotator AddShooterSpin_Internal(FShooterStatus InStatus, float DeltaTime);
	UPROPERTY()
	float LastPitchRotation = 0.f;
	UPROPERTY(EditAnywhere, Category=MouseMovement)
	float AirPitchSpeed = 2.f;
	UPROPERTY(EditAnywhere, Category = MouseMovement)
	float MaxPitchSpeed = 5.f;
	
	FRotator YawLook_Internal(FShooterStatus& OutStatus, float DeltaTime);
	float LastYawRotation = 0.f;
	UPROPERTY(EditAnywhere, Category=MouseMovement)
	float AirRotationSpeed = 0.25f;
	UPROPERTY(EditAnywhere, Category=MouseMovement)
	float AirRotationMaxSpeed = 2.f;
	/**
	* @end 
	*/



	//everything involved with gravity
	FTransform PerformGravity(FShooterStatus& OutStatus, float DeltaTime) const;
	
	void FindClosestFloor(FTransform ActorTransform, FShooterStatus& OutStatus) const;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float SphereTraceRadius = 750.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float GravityDistanceRadius = 2500.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float ImpactEdgeAdjustment = 5.f;
	
	FRotator OrientToGravity(FRotator InActorRotation, FShooterStatus InStatus, float DeltaTime) const;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float SlerpSpeed = 1000.f;
	
	FVector GravityForce(FVector InActorLocation, FShooterStatus& OutStatus, float DeltaTime) const;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float OutRangeGravityStrength = 0.3f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float InRangeGravityStrength = 500.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float GravityForceCurve = 2.f;
	UPROPERTY(EditAnywhere, Category = Gravity)
	float GravityVelocityReduction = 1.15f;
	/**
	 * @end 
	 */

	//everything to do with jumping
	void JumpPressed(const FInputActionValue& ActionValue);
	
	void BuildJump(FShooterMove& OutMove);
	bool bJumpPressed = false;
	
	FVector Jump_Internal(bool bJumpWasPressed, FShooterStatus& OutStatus, float DeltaTime);
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpVelocity = 10.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpDeceleration = 10.f;
	/**
	 * @end
	 */

	//everything involved with magnetizing
	void BuildMagnetized(FShooterMove& OutMove);
	void MagnetizePressed(const FInputActionValue& ActionValue);
	bool bMagnetizedPressed = false;
	void Magnetize_Internal(bool bMagnetizedFromMove, FShooterStatus& OutStatus);
	/**
	 * @end
	 */

	//everything involved with boosting
	void BoostPressed(const FInputActionValue& ActionValue);
	
	void BuildBoost(FShooterMove& OutMove);
	FVector BoostDirection;
	bool bBoostPressed = false;
	
	void Boost_Internal(FVector BoostVector, bool bBoostWasPressed, bool bIsPredictiveState, FShooterStatus& OutStatus);
	UPROPERTY(EditAnywhere, Category=Boost)
	float BoostLastVelocityReduction = 1.15f;
	
	void ContactedBoostForce(const FVector BoostVector, FShooterStatus& OutStatus) const;
	UPROPERTY(EditAnywhere, Category=Boost)
	float NonContactedBoostSpeed = 25.f;
	UPROPERTY(EditAnywhere, Category=Boost)
	float ContactedBoostSpeed = 30.f;
	UPROPERTY(EditAnywhere, Category=Boost)
	float MagnetizeDelay = 1.f;
	UPROPERTY(EditAnywhere, Category=Boost)
	float BoostRechargeRate = 5.f;
	UPROPERTY(EditAnywhere, Category = Boost)
	int8 MaxBoosts = 2;

	void BoostRechargeConfirmed();
	void BoostRechargePredictive();
	FTimerHandle BoostRechargeTimerHandle;
	/**
	 * @end 
	 */
	
	UFUNCTION()
	void PassDamageToHealth(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	void ZeroOutGravity(FShooterStatus& StatusToReset);

	
	
public:
	FORCEINLINE EShooterFloorStatus GetFloorStatus() const {return LocalStatus.ShooterFloorStatus;}
	EShooterFloorStatus SetFloorStatus(EShooterFloorStatus StatusToChangeTo, FShooterStatus& StatusToReset);
	float GetSpringArmPitch() const;
	bool GetIsMagnetized() const;
	// FORCEINLINE void SetIsMagnetized(bool bMagnetization) { bIsMagnetized = bMagnetization; }
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Skeleton; }
	FVector GetHitTarget();
	FORCEINLINE UShooterCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE UShooterHealthComponent* GetHealthComponent() const { return Health; }
};

