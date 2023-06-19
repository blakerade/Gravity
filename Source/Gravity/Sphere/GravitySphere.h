// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GravitySphere.generated.h"

class USphereComponent;
UCLASS()
class GRAVITY_API AGravitySphere : public AStaticMeshActor
{
	GENERATED_BODY()
	
public:	
	AGravitySphere();
	void BeginPlayAddSphereLevel();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	USphereComponent* OverlapSphere;

	UFUNCTION()
	void AddSphereLevel(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UPROPERTY(EditAnywhere, Category = Gravity)
	float GravityStrength = 10000.f;

private:
	
public:	


};
