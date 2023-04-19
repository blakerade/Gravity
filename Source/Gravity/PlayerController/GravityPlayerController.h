// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GravityPlayerController.generated.h"

class AShooterHUD;

/**
 * 
 */
UCLASS()
class GRAVITY_API AGravityPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterHUD* ShooterHUD;
	
protected:
	virtual void BeginPlay() override;
	
	
private:
	
	
public:
};
