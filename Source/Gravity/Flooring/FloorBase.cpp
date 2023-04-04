// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorBase.h"

#include "Components/BoxComponent.h"
#include "Gravity/Characters/BasePawnPlayer.h"

AFloorBase::AFloorBase()
{
	GravityTrigger = CreateDefaultSubobject<UBoxComponent>("Gravity Trigger");
	GravityTrigger->SetupAttachment(RootComponent);
	BottomGravityTrigger = CreateDefaultSubobject<UBoxComponent>("BottomGravityTrigger");
	BottomGravityTrigger->SetupAttachment(RootComponent);
}

void AFloorBase::BeginPlay()
{
	Super::BeginPlay();

	if(GravityTrigger)
	{
		GravityTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFloorBase::SetPawnGravity);
		GravityTrigger->OnComponentEndOverlap.AddDynamic(this, &AFloorBase::RemovePawnGravity);
		BottomGravityTrigger->OnComponentBeginOverlap.AddDynamic(this, &AFloorBase::SetPawnGravityForBottom);
		BottomGravityTrigger->OnComponentEndOverlap.AddDynamic(this, &AFloorBase::RemovePawnGravityForBottom);
	}
	FlooringGravity = RootComponent->GetUpVector() * -GravityStrength;
	
}

void AFloorBase::SetPawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->AddToGravities(FlooringGravity);
	}
}

void AFloorBase::SetPawnGravityForBottom(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->AddToGravities(-FlooringGravity);
	}
}

void AFloorBase::RemovePawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->RemoveFromGravities(FlooringGravity);
		if(PawnPlayer->GetGravitiesSize() == 0)
		{
			PawnPlayer->SetContactedWith(false);
			PawnPlayer->SetMagnetization(false);
		}
	}
}

void AFloorBase::RemovePawnGravityForBottom(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->RemoveFromGravities(-FlooringGravity);
		if(PawnPlayer->GetGravitiesSize() == 0)
		{
			PawnPlayer->SetContactedWith(false);
			PawnPlayer->SetMagnetization(false);
		}
	}
}


