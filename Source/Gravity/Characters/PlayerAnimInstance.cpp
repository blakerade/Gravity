// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include <string>

#include "BasePawnPlayer.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Player = Cast<ABasePawnPlayer>(TryGetPawnOwner());
}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	Player = Player == nullptr ? Cast<ABasePawnPlayer>(TryGetPawnOwner()) : Player;

	if(Player)
	{
		Pitch = Player->GetSpringArmPitch();
		bOnAFloor = Player->GetContactedWith();
		if(bOnAFloor)
		{
			FTransform WorldTransform = Player->GetTransform();
			FVector LocalVeclotiy = WorldTransform.InverseTransformVector(Player->GetVelocity());
			if(!FMath::IsNearlyZero(LocalVeclotiy.X, 0.2)) ForwardSpeed = LocalVeclotiy.X;
			else ForwardSpeed = 0.f;
			if(!FMath::IsNearlyZero(LocalVeclotiy.Y, 0.2)) LateralSpeed = LocalVeclotiy.Y;
			else LateralSpeed = 0.f;
			bMagnitized = false;
		}
		if(!bOnAFloor)
		{
			bMagnitized = Player->GetIsMagnitized();
		}
	}
}
