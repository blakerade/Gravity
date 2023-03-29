// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorBase.h"

#include "Components/BoxComponent.h"
#include "Gravity/Characters/BasePawnPlayer.h"

AFloorBase::AFloorBase()
{
	GravityTrigger = CreateDefaultSubobject<UBoxComponent>("Gravity Trigger");
	GravityTrigger->SetupAttachment(RootComponent);
}

void AFloorBase::BeginPlay()
{
	Super::BeginPlay();

	if(GravityTrigger)
	{
		GravityTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFloorBase::SetPawnGravity);
	}
	FlooringGravity = RootComponent->GetUpVector() * -GravityStrength;
	
}

void AFloorBase::SetPawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Inside SetPawnGravity 1"));
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Inside SetPawnGravity 2"));
		PawnPlayer->SetCurrentGravity(FlooringGravity);
	}
}


