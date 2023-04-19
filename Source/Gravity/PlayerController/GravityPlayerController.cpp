// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityPlayerController.h"
#include "Gravity/HUD/ShooterHUD.h"

void AGravityPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	ShooterHUD = Cast<AShooterHUD>(GetHUD());
	if(ShooterHUD)
	{
		ShooterHUD->AddShooterOverlay();
	}
}
