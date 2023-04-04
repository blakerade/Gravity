// Fill out your copyright notice in the Description page of Project Settings.


#include "SphereFloorBase.h"
#include "Gravity/Characters/BasePawnPlayer.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

ASphereFloorBase::ASphereFloorBase()
{
	GravityTrigger->DestroyComponent();
	BottomGravityTrigger->DestroyComponent();
	SphereGravityTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("GravityTrigger"));
	SphereGravityTrigger->SetupAttachment(RootComponent);
}

void ASphereFloorBase::BeginPlay()
{
	FlooringGravity = GetActorLocation();
	SphereGravityTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASphereFloorBase::SetPawnGravity);
	SphereGravityTrigger->OnComponentEndOverlap.AddDynamic(this, &ASphereFloorBase::RemovePawnGravity);
}

void ASphereFloorBase::SetPawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->AddToSpheres(this);
		UE_LOG(LogTemp, Warning, TEXT("Added"));
	}
}

void ASphereFloorBase::RemovePawnGravity(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->RemoveFromSphere(this);
		UE_LOG(LogTemp, Warning, TEXT("Removed"));
		if(PawnPlayer->GetSpheresSize() == 0)
		{
			PawnPlayer->SetContactedWith(false);
			PawnPlayer->SetMagnetization(false);
		}
	}
}
