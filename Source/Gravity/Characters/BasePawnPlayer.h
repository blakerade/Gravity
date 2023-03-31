// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "BasePawnPlayer.generated.h"

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
	float GroundSpeed = 100.f;
	
	void AirMove(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirSpeed = 10.f;
	
	void Look(const FInputActionValue& ActionValue);
	
	void Jump(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpVelocity = 100.f;
	
	void Crouch(const FInputActionValue& ActionValue);
	
	void Magnetize(const FInputActionValue& ActionValue);
	
	bool bIsMagnetized = false;
	
	void Boost(const FInputActionValue& ActionValue);
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float BoostSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category=Movement)
	float FloorFriction = 3.f;
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float AirFriction = 0.01f;

	virtual void PreformPlayerMovement();
	
	virtual void PreformGravity(float DeltaTime);
	UPROPERTY(EditAnywhere, Category=Gravity)
	float SphereGravityStrength = 10000.f;
	
	UFUNCTION()
	void OnFloorHit(UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);
	

private:
	UPROPERTY(VisibleAnywhere)
	FVector CurrentGravity = FVector(0.f, 0.f, 0.f);

	bool bContactedWithFloor = false;
	bool bContactedWithSphere = false;
	
	FMatrix FeetToGravity;

	bool bIsInsideSphere = false;

	UPROPERTY()
	AGravitySphere* Sphere;
	FVector SphereCenter;
	void FindSphere();

	void OrientToGravity(FVector CurrentGravity, float DeltaTime);
	
public:
	void SetCurrentGravity(FVector InGravity) { CurrentGravity = InGravity;}
	void SetIsInSphere(bool bWithinSphere) {bIsInsideSphere = bWithinSphere;}
	void SetSphere(AGravitySphere* LevelSphere) {Sphere = LevelSphere;}
	void SetSphereCenter(FVector SphereLocation) {SphereCenter = SphereLocation;}
};
