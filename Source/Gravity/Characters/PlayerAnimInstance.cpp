// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

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
		Speed = Player->GetVelocity().Size();
	}
}
