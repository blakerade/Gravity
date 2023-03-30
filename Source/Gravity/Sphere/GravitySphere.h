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


protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	USphereComponent* GravityTrigger;

	UFUNCTION()
	virtual void SetPawnIsInSphere(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult & SweepResult);

	UFUNCTION()
	virtual void SetPawnIsOutOfSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(EditAnywhere)
	FVector FlooringGravity;

	UPROPERTY(EditAnywhere)
	float GravityStrength = 10000.f;

private:
	
public:	


};
