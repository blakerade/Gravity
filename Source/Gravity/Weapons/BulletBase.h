// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletBase.generated.h"

class UProjectileMovementComponent;
class UBoxComponent;
UCLASS()
class GRAVITY_API ABulletBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletBase();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	UBoxComponent* BulletBox;

	UPROPERTY(EditAnywhere)
	UProjectileMovementComponent* BulletMovement;

	UFUNCTION()
	void OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	UPROPERTY(EditAnywhere)
	float BulletDamage = 20.f;

public:	


};
