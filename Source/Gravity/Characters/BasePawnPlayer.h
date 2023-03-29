// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "BasePawnPlayer.generated.h"

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

	virtual void PreformPlayerMovement();
	virtual void PerformGravity(float DeltaTime);
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
	FMatrix OrientToGravity;
	
public:
	void SetCurrentGravity(FVector InGravity) { CurrentGravity = InGravity;}
};
