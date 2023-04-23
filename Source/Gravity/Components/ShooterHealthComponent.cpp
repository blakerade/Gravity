// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHealthComponent.h"


UShooterHealthComponent::UShooterHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}


// Called when the game starts
void UShooterHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


// Called every frame
void UShooterHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

