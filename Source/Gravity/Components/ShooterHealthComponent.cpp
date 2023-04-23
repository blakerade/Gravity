// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHealthComponent.h"

#include "Gravity/Characters/BasePawnPlayer.h"
#include "Gravity/PlayerController/GravityPlayerController.h"
#include "Gravity/Sphere/GravitySphere.h"


UShooterHealthComponent::UShooterHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}


// Called when the game starts
void UShooterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Shooter = Cast<ABasePawnPlayer>(GetOwner());
	if(Shooter)
	{
		ShooterController = Cast<AGravityPlayerController>(Shooter->Controller);
	}
}

void UShooterHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	Shooter = Shooter == nullptr ? Cast<ABasePawnPlayer>(GetOwner()) : Shooter;
	if(Shooter)
	{
		ShooterController = ShooterController == nullptr ? Cast<AGravityPlayerController>(Shooter->Controller) : ShooterController;
		if(ShooterController)
		{
			Health = FMath::Max(Health - Damage, 0.f);
			ShooterController->UpdateShooterHUDHealth();
		}
	}
}


// Called every frame
void UShooterHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

