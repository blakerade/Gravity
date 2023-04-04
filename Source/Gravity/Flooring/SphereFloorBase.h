// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FloorBase.h"
#include "SphereFloorBase.generated.h"

class USphereComponent;
/**
 * 
 */
UCLASS()
class GRAVITY_API ASphereFloorBase : public AFloorBase
{
	GENERATED_BODY()

public:
	ASphereFloorBase();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	USphereComponent* SphereGravityTrigger;
	
	virtual void SetPawnGravity(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult & SweepResult);
	
	virtual void RemovePawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
};
