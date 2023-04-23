// Fill out your copyright notice in the Description page of Project Settings.


#include "GravityPlayerController.h"

#include "Components/ProgressBar.h"
#include "Gravity/Characters/BasePawnPlayer.h"
#include "Gravity/Components/ShooterHealthComponent.h"
#include "Gravity/HUD/ShooterHUD.h"
#include "Gravity/HUD/UShooterOverlay.h"

void AGravityPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterCharacter = Cast<ABasePawnPlayer>(GetOwner());
	ShooterHUD = Cast<AShooterHUD>(GetHUD());
	if(ShooterHUD)
	{
		ShooterHUD->AddShooterOverlay();
		SetHUDHealth();
	}
}

void AGravityPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PollInit();
}

void AGravityPlayerController::SetHUDHealth()
{
	ShooterCharacter = ShooterCharacter == nullptr ? Cast<ABasePawnPlayer>(GetPawn()) : ShooterCharacter;
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bUpdateHealth = ShooterCharacter &&
		ShooterCharacter->GetHealthComponent() &&
		ShooterHUD &&
		ShooterHUD->ShooterOverlay && 
		ShooterHUD->ShooterOverlay->HealthBar;
	if(bUpdateHealth)
	{
		ShooterHUD->ShooterOverlay->HealthBar->SetPercent(ShooterCharacter->GetHealthComponent()->Health / ShooterCharacter->GetHealthComponent()->MaxHealth);
		bHUDHealthSet = true;
	}
}

void AGravityPlayerController::UpdateShooterHUDHealth()
{
	ShooterCharacter = ShooterCharacter == nullptr ? Cast<ABasePawnPlayer>(GetPawn()) : ShooterCharacter;
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bUpdateHealth = ShooterCharacter &&
		ShooterCharacter->GetHealthComponent() &&
		ShooterHUD &&
		ShooterHUD->ShooterOverlay && 
		ShooterHUD->ShooterOverlay->HealthBar;
	if(bUpdateHealth)
	{
		ShooterHUD->ShooterOverlay->HealthBar->SetPercent(ShooterCharacter->GetHealthComponent()->Health / ShooterCharacter->GetHealthComponent()->MaxHealth);
		// if(!bHUDHealthSet) bHUDHealthSet = true;
	}
}

void AGravityPlayerController::PollInit()
{
	if(!bHUDHealthSet)
	{
		UpdateShooterHUDHealth();
	}
}
