// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "BasePawnPlayer.h"
#include "Gravity/Weapons/WeaponBase.h"


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
		bOnAFloor = static_cast<int>(Player->GetFloorStatus()) > 0 ? true : false;	
		if(bOnAFloor)
		{
			FTransform WorldTransform = Player->GetTransform();
			FVector LocalVeclotiy = WorldTransform.InverseTransformVector(Player->GetVelocity());
			if(!FMath::IsNearlyZero(LocalVeclotiy.X, 50.f)) ForwardSpeed = LocalVeclotiy.X;
			else ForwardSpeed = 0.f;
			if(!FMath::IsNearlyZero(LocalVeclotiy.Y, 50.f)) LateralSpeed = LocalVeclotiy.Y;
			else LateralSpeed = 0.f;
		}
		bMagnetize = Player->GetIsMagnetized();
	}
}
