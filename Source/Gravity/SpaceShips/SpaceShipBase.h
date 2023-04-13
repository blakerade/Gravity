// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "SpaceShipBase.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UFloatingPawnMovement;
class UInputAction;
class UInputMappingContext;

UCLASS()
class GRAVITY_API ASpaceShipBase : public APawn
{
	GENERATED_BODY()

public:
	ASpaceShipBase();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ShipBody;
	UPROPERTY(EditAnywhere)
	USpringArmComponent* SpringArm;
	UPROPERTY(EditAnywhere)
	UCameraComponent* Camera;
	
	void Move(const FInputActionValue& ActionValue);
	void Yaw(const FInputActionValue& ActionValue);
	UPROPERTY(EditAnywhere)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere)
	UInputAction* YawAction;
	UPROPERTY(EditAnywhere)
	UInputMappingContext* ShipMapping;

private:
	UPROPERTY(EditAnywhere)
	float ThusterSpeed = 100.f;
	UPROPERTY(EditAnywhere)
	float BrakeSpeed = 50.f;
	UPROPERTY(EditAnywhere)
	float PitchSpeed = 5.f;
	UPROPERTY(EditAnywhere)
	float RollSpeed = 5.f;
	UPROPERTY(EditAnywhere)
	float YawSpeed = 1.f;

public:	
	

};
