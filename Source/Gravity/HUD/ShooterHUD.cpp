// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHUD.h"

#include "UShooterOverlay.h"
#include "Blueprint/UserWidget.h"

void AShooterHUD::AddShooterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if(PC && ShooterOverlayClass)
	{
		ShooterOverlay = CreateWidget<UShooterOverlay>(PC,ShooterOverlayClass);
		if(ShooterOverlay)
		{
			ShooterOverlay->AddToViewport();
		}
	}
}
