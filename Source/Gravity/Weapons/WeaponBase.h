// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class ABulletBase;
class UWidgetComponent;
class USphereComponent;
class ABasePawnPlayer;

UCLASS()
class GRAVITY_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();
	virtual void Tick(float DeltaTime) override;
	//Crosshairs
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairTop;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairBottom;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairRight;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairLeft;
	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairCenter;
	
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABulletBase> BulletClass;

private:
	UPROPERTY()
	ABasePawnPlayer* Shooter;
	
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* WeaponBodyMesh;

	UPROPERTY(EditAnywhere)
	USphereComponent* WeaponPickupSphere;

	UFUNCTION()
	void ShowPickupWidget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	UFUNCTION()
	void HidePickupWidget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:	
	void RequestFire(FVector HitTarget);

};
