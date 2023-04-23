// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UShooterOverlay.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * 
 */
UCLASS()
class GRAVITY_API UShooterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PickupText;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	
protected:

private:

public:
	
	
};
