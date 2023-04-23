// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterCombatComponent.generated.h"

class AShooterHUD;
class AGravityPlayerController;
class AWeaponBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAVITY_API UShooterCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterCombatComponent();
	friend class ABasePawnPlayer;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	ABasePawnPlayer* Shooter;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeaponBase> DefaultWeaponClass;
	UPROPERTY()
	AWeaponBase* DefaultWeapon;
	UPROPERTY()
	AWeaponBase* EquippedWeapon;
	UPROPERTY()
	AShooterHUD* ShooterHUD;
	UPROPERTY()
	AGravityPlayerController* PC;
	
private:
	void SetHUDCrossHairs();
	void TraceUnderCrosshairs(FHitResult HitResult);
	FVector HitTarget;
	
	
public:	
	
};
