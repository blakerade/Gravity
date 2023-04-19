// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

class UShooterOverlay;
/**
 * 
 */
UCLASS()
class GRAVITY_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	void AddShooterOverlay();
	UShooterOverlay* ShooterOverlay;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ShooterOverlayClass;

	

	
private:

public:
	
};
