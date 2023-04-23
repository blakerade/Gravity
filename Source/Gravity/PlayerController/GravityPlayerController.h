// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GravityPlayerController.generated.h"

class ABasePawnPlayer;
class AShooterHUD;

/**
 * 
 */
UCLASS()
class GRAVITY_API AGravityPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UPROPERTY()
    AShooterHUD* ShooterHUD;
    UPROPERTY()
    ABasePawnPlayer* ShooterCharacter;
	void UpdateShooterHUDHealth();
	
protected:
	virtual void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	
private:
	void SetHUDHealth();
	bool bHUDHealthSet = false;
	void PollInit();
	
public:
};
