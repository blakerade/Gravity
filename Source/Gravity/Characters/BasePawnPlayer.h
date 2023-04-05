// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "BasePawnPlayer.generated.h"

struct FHUDPackage;
class ASphereFloorBase;
class AGravitySphere;
class UCameraComponent;
class USpringArmComponent;
class UCapsuleComponent;
class UInputAction;
class UInputMappingContext;

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
	
	//Components
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* Capsule;
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* Skeleton;
	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;

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
	UInputMappingContext* CharacterMovementMapping;

	//Input Functions
	void Move(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float GroundSpeed = 5000.f;

	UPROPERTY(EditAnywhere, Category=Movement)
	float ForwardSpeed = 1.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BackwardsSpeed = 1.f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float LateralSpeed = 1.f;
	
	void AirMove(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirSpeed = 100.f;
	
	void Look(const FInputActionValue& ActionValue);

	UPROPERTY(EditAnywhere, Category=Movement)
	float AirForwardRollSpeed = 10.f;
	
	void Jump(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpVelocity = 25000.f;
	
	void Crouch(const FInputActionValue& ActionValue);
	
	void Magnetize(const FInputActionValue& ActionValue);
	
	bool bIsMagnetized = false;
	
	void Boost(const FInputActionValue& ActionValue);
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostCurrentVelocityReduction = 2.5f;
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostSpeed = 50000.f;
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

	virtual void PerformPlayerMovement();
	
	virtual void PerformGravity(float DeltaTime);
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereGravityStrength = 10000.f;
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereFloorGravityStrength = 3000.f;
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereFloorContactedForceCorrection = 3000.f;
	
	UFUNCTION()
	void OnFloorHit(UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
	

private:
	UPROPERTY(VisibleAnywhere)
	FVector CurrentGravity = FVector(0.f, 0.f, 0.f);

	TArray<FVector> Gravities;
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
	void FindClosestGravity(float& OutDistanceToGravity);
	void IsThereACloserSphereFloor(float GravityDistanceCheck, bool& OutSphereFloorOverride);
	void OrientToGravity(FVector CurrentGravity, float DeltaTime);
	void SphereFloorContactedGravity(float DeltaTime);

	
public:
	// void SetCurrentGravity(FVector InGravity) { CurrentGravity = InGravity;}
	void SetIsInSphere(bool bWithinSphere) {bIsInsideSphere = bWithinSphere;}
	// void SetSphere(AGravitySphere* LevelSphere) {Sphere = LevelSphere;}
	// void SetSphereCenter(FVector SphereLocation) {SphereCenter = SphereLocation;}
	void SetContactedWith(bool bIsContactedWith);
	void SetMagnetization(bool bMagnetization) {bIsMagnetized = bMagnetization;}
	bool GetContactedWith() {return bContactedWithFloor || bContactedWithLevelSphere;}
	int32 GetGravitiesSize() {return Gravities.Num(); }
	void AddToGravities(FVector GravityToAdd) {Gravities.Add(GravityToAdd);}
	void RemoveFromGravities(FVector GravityToRemove) 	{Gravities.Remove(GravityToRemove);}
	int32 GetSpheresSize() {return SphereFloors.Num(); }
	void AddToSpheres(ASphereFloorBase* SphereToAdd) {SphereFloors.Add(SphereToAdd);}
	void RemoveFromSphere(ASphereFloorBase* SphereToRemove) {SphereFloors.Remove(SphereToRemove); }
	float GetSpringArmPitch() { return SpringArm->GetRelativeRotation().Pitch; } 
};
