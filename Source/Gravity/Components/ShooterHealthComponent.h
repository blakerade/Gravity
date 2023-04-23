// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShooterHealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GRAVITY_API UShooterHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UShooterHealthComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere)
	float Health = 100.f;

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;
	
protected:
	virtual void BeginPlay() override;

private:

	
public:	
	
};
