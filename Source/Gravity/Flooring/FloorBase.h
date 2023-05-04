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
	
private:


public:
	
};
