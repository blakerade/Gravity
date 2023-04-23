// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class ABasePawnPlayer;
/**
 * 
 */
UCLASS()
class GRAVITY_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY()
	ABasePawnPlayer* Player;

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool bOnAFloor;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	float ForwardSpeed;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	float LateralSpeed;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	float Yaw;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	float Pitch;
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess))
	bool bMagnetize;

public:
};
