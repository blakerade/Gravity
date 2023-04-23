// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterHealthComponent.generated.h"


class AGravityPlayerController;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAVITY_API UShooterHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABasePawnPlayer;
	UShooterHealthComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	float Health = 100.f;

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;

protected:
	virtual void BeginPlay() override;

private:
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
	UPROPERTY()
	ABasePawnPlayer* Shooter;
	UPROPERTY()
	AGravityPlayerController* ShooterController;
	
public:	
	
};
