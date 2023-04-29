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
	FVector MovementVector;
	UPROPERTY()
	FVector2D PitchVector;
	UPROPERTY()
	bool bJumped = false;
	UPROPERTY()
	bool bMagnetized = false;
	UPROPERTY()
	bool bDidBoost = false;
	UPROPERTY()
	FVector BoostDirection;
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
	FShooterFloorStatus FloorStatus = FShooterFloorStatus::NoFloorContact;
	
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
	UPROPERTY(Replicated)
	float SpringArmClientPitch;
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
	

	//Input Functions
	void Move(const FInputActionValue& ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_Move(FVector ActionValue);
	void Move_Internal(FVector ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundSpeed = 5000.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float ForwardSpeed = 1.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BackwardsSpeed = 0.5f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float LateralSpeed = 0.5f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirSpeed = 200.f;
	
	void Look(const FInputActionValue& ActionValue);
	void Look_Internal(FVector2D ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_Look(FVector2D ActionValue);
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirRotationSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category=Movement)
	float AirForwardRollSpeed = 190000.f;
	
	void Jump(const FInputActionValue& ActionValue);
	void Jump_Internal(float ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_Jump(float ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpVelocity = 40000.f;
	
	void Crouch(const FInputActionValue& ActionValue);
	
	void Magnetize(const FInputActionValue& ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_Magnetize();
	void Magnetize_Internal();
	UPROPERTY(Replicated)
	bool bIsMagnetized = false;

	void BoostWithDirection(const FInputActionValue& ActionValue);
	void Boost(const FInputActionValue& ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_Boost(FVector BoostDirection);
	void Boost_Internal(FVector BoostDirection);
	void BoostForce(const FVector BoostDirection);
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
	void Fire(const FInputActionValue& ActionValue);

	virtual void PerformPlayerMovement();
	
	virtual void PerformGravity(float DeltaTime);
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereGravityStrength = 3000.f;
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereFloorGravityStrength = 30.f;
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereFloorContactedForceCorrection = 300000.f;
	
	UFUNCTION()
	void OnFloorHit(UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float KnockBackImpulse = 1.75f;

private:
	UPROPERTY(VisibleAnywhere)
	FVector CurrentGravity = FVector(0.f, 0.f, 0.f);
	bool bHaveAGravity = false;

	TArray<AFloorBase*> FloorGravities;
	TArray<ASphereFloorBase*> SphereFloors;
	
	UPROPERTY()
	ASphereFloorBase* SphereContactedWith;
	bool bContactedWithLevelSphere = false;
	
	FMatrix FeetToGravity;

	bool bIsInsideSphere = false;

	UPROPERTY()
	AGravitySphere* Sphere;
	FVector SphereCenter;
	void FindSphere();
	void FindClosestGravity(float& OutDistanceToGravity, bool& OutIsAFloorBase);
	void IsThereACloserSphereFloor(bool bHaveAFloorBase, float& InAndOutGravityDistanceCheck, bool& OutSphereFloorOverride);
	void OrientToGravity(FVector CurrentGravity, float DeltaTime, float DistanceToGravity, bool bIsThereAFloor);
	UPROPERTY(EditAnywhere, Category = Movement)
	float RotationSpeedDampener = 100.f;
	void SphereFloorContactedGravity(float DeltaTime);
	UFUNCTION()
	void PassDamageToHealth(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION(Server, Unreliable)
	void SendServerMove(FShooterMove ClientMove);

	TArray<FShooterMove> UnacknowledgedMoves;
	UPROPERTY(ReplicatedUsing = OnRep_StatusOnServer)
	FShooterStatus StatusOnServer;

	
	UFUNCTION()
	void OnRep_StatusOnServer();
	void ClearAcknowledgedMoves();
	void PlayUnacknowledgedMoves();
	
	
public:
	FORCEINLINE void SetIsInSphere(bool bWithinSphere) {bIsInsideSphere = bWithinSphere;}
	FORCEINLINE void ZeroOutCurrentGravity() {CurrentGravity = FVector::ZeroVector, bHaveAGravity = false;}
	FORCEINLINE FShooterFloorStatus GetFloorStatus() const {return FloorStatus;}
	void SetFloorStatus(FShooterFloorStatus InFloorStatus);
	FORCEINLINE int32 GetGravitiesSize() const {return FloorGravities.Num(); }
	FORCEINLINE void AddToFloorGravities(AFloorBase* GravityToAdd) {FloorGravities.Add(GravityToAdd);}
	FORCEINLINE void RemoveFromFloorGravities(AFloorBase* GravityToRemove) 	{FloorGravities.Remove(GravityToRemove);}
	FORCEINLINE int32 GetSpheresSize() const {return SphereFloors.Num(); }
	FORCEINLINE void AddToSpheres(ASphereFloorBase* SphereToAdd) {SphereFloors.Add(SphereToAdd);}
	FORCEINLINE void RemoveFromSphere(ASphereFloorBase* SphereToRemove) {SphereFloors.Remove(SphereToRemove); }
	float GetSpringArmPitch() const;
	FORCEINLINE bool GetIsMagnetized() const {return bIsMagnetized;}
	FORCEINLINE void SetHaveAGravity(bool ResetGravity) { bHaveAGravity = ResetGravity; }
	FORCEINLINE void SetbIsMagnetized(bool bMagnetization) { bIsMagnetized = bMagnetization; }
	FORCEINLINE USkeletalMeshComponent* GetMesh() const { return Skeleton; }
	FVector GetHitTarget();
	FORCEINLINE UShooterCombatComponent* GetCombatComponent() const { return Combat; }
	FORCEINLINE UShooterHealthComponent* GetHealthComponent() const { return Health; }
};
