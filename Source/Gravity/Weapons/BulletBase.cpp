// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletBase.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Gravity/Characters/BasePawnPlayer.h"
#include "Kismet/GameplayStatics.h"

ABulletBase::ABulletBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	BulletBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bullet"));
	SetRootComponent(BulletBox);
	BulletMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
}

void ABulletBase::BeginPlay()
{
	Super::BeginPlay();

	//this isn't setting it, putting it in tick does. might need to make a pollinit in the future if shooting yourself becomes a problem
	BulletBox->MoveIgnoreActors.Add(GetOwner());
	BulletBox->OnComponentHit.AddDynamic(this, &ABulletBase::OnBulletHit);
}

void ABulletBase::OnBulletHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABasePawnPlayer* Shooter = Cast<ABasePawnPlayer>(OtherActor);
	if(Shooter)
	{
		UGameplayStatics::ApplyDamage(OtherActor, BulletDamage, Shooter->Controller, this,  UDamageType::StaticClass());
	}
	Destroy();
}

void ABulletBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

