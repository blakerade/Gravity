// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

class UTexture2D;
class UShooterOverlay;
/**
 * 
 */

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor;
	
};

UCLASS()
class GRAVITY_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	void AddShooterOverlay();
	virtual void DrawHUD() override;
	UShooterOverlay* ShooterOverlay;
	FHUDPackage* HUDPackage;
	
protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> ShooterOverlayClass;
	
private:

public:
	
};
