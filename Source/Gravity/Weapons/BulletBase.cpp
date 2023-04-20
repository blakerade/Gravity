// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletBase.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = true;

	BulletBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bullet"));
	SetRootComponent(BulletBox);
	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
}

void ABulletBase::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ABulletWasBorn!"));
}

void ABulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

