// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BasePawnPlayer.generated.h"

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

UCLASS()
class GRAVITY_API ABasePawnPlayer : public APawn
{
	GENERATED_BODY()

public:
	ABasePawnPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	
	//Hitboxes
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

	//Inputs Bindings
	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere)
	UInputAction* AirMoveAction;
	UPROPERTY(EditAnywhere)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere)
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere)
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere)
	UInputAction* MagnetizeAction;
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
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundSpeed = 5000.f;

	UPROPERTY(EditAnywhere, Category=Movement)
	float ForwardSpeed = 1.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BackwardsSpeed = 0.5f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float LateralSpeed = 0.5f;
	
	void AirMove(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirSpeed = 200.f;
	
	void Look(const FInputActionValue& ActionValue);
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirRotationSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category=Movement)
	float AirForwardRollSpeed = 190000.f;
	
	void Jump(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpVelocity = 40000.f;
	
	void Crouch(const FInputActionValue& ActionValue);
	
	void Magnetize(const FInputActionValue& ActionValue);
	
	bool bIsMagnetized = false;
	
	void Boost(const FInputActionValue& ActionValue);
	void Boost_Internal();
	void ContactedFloorMagnitizeDelay();
	FTimerHandle MagnitizeDealyForBoost;
	UPROPERTY(EditAnywhere, Category=Movement)
	float MagnitizeDelay = 3.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostCurrentVelocityReduction = 2.5f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostSpeed = 55000.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostRechargeRate = 5.f;
	bool bCanBoost = true;
	int8 BoostCount = 0;
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

	bool bContactedWithFloor = false;
	bool bContactedWithSphereFloor = false;
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

	
public:
	// void SetCurrentGravity(FVector InGravity) { CurrentGravity = InGravity;}
	void SetIsInSphere(bool bWithinSphere) {bIsInsideSphere = bWithinSphere;}
	// void SetSphere(AGravitySphere* LevelSphere) {Sphere = LevelSphere;}
	// void SetSphereCenter(FVector SphereLocation) {SphereCenter = SphereLocation;}
	void SetContactedWith(bool bIsContactedWith);
	void ZeroOutCurrentGravity() {CurrentGravity = FVector::ZeroVector, bHaveAGravity = false;}
	bool GetContactedWith() {return bContactedWithFloor || bContactedWithLevelSphere || bContactedWithSphereFloor;}
	int32 GetGravitiesSize() {return FloorGravities.Num(); }
	void AddToFloorGravities(AFloorBase* GravityToAdd) {FloorGravities.Add(GravityToAdd);}
	void RemoveFromFloorGravities(AFloorBase* GravityToRemove) 	{FloorGravities.Remove(GravityToRemove);}
	int32 GetSpheresSize() {return SphereFloors.Num(); }
	void AddToSpheres(ASphereFloorBase* SphereToAdd) {SphereFloors.Add(SphereToAdd);}
	void RemoveFromSphere(ASphereFloorBase* SphereToRemove) {SphereFloors.Remove(SphereToRemove); }
	float GetSpringArmPitch() { return SpringArm->GetRelativeRotation().Pitch; }
	bool GetIsMagnitized() {return bIsMagnetized;}
	void SetHaveAGravity(bool ResetGravity) { bHaveAGravity = ResetGravity; }
	void SetbIsMagnitized(bool bMagnetization) { bIsMagnetized = bMagnetization; }
	USkeletalMeshComponent* GetMesh() { return Skeleton; }
};
