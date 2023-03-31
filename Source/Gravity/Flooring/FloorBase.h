// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "FloorBase.generated.h"


class UBoxComponent;
/**
 * 
 */
UCLASS()
class GRAVITY_API AFloorBase : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	AFloorBase();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* GravityTrigger;
	UPROPERTY(EditAnywhere)
	UBoxComponent* BottomGravityTrigger;
	
	UFUNCTION()
	virtual void SetPawnGravity(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult & SweepResult);

	UFUNCTION()
	virtual void RemovePawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(EditAnywhere)
	FVector FlooringGravity;

	UPROPERTY(EditAnywhere)
	float GravityStrength = 10000.f;
	
private:
	
public:
	
};
