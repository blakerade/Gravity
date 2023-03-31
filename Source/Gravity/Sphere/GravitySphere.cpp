// Fill out your copyright notice in the Description page of Project Settings.


#include "GravitySphere.h"

#include "Components/SphereComponent.h"
#include "Gravity/Characters/BasePawnPlayer.h"
#include "Kismet/KismetSystemLibrary.h"

AGravitySphere::AGravitySphere()
{
	PrimaryActorTick.bCanEverTick = true;

	GravityTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("TriggerSphere"));
	GravityTrigger->SetupAttachment(RootComponent);

}

void AGravitySphere::BeginPlay()
{
	Super::BeginPlay();

	//Check for pawns inside the sphere so that we can give them a gravity if/when they spawn inside of it
	//The parameters needed for SphereOverlapActors //////////////////////////////////////////////////////
	TArray<TEnumAsByte<EObjectTypeQuery>> PawnTypes;
	PawnTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	TArray<AActor*> OutActors;
	UClass* SeekClass = UClass::StaticClass();
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	
	UKismetSystemLibrary::SphereOverlapActors(this, GetActorLocation(), GravityTrigger->GetScaledSphereRadius(), PawnTypes, SeekClass, IgnoreActors, OutActors);
	//Set bIsInSphere on all pawns within Sphere to true and give them object reference  for gravity
	for(AActor* Player: OutActors)
	{
		if(ABasePawnPlayer* PlayerToSetInSphere = Cast<ABasePawnPlayer>(Player))
		{
			PlayerToSetInSphere->SetIsInSphere(true);
		}
	}

	//If for whatever reason something begins overlap with gravity trigger, set bIsInSphere
	GravityTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGravitySphere::SetPawnIsInSphere);
	//And remove it if they leave the sphere
	GravityTrigger->OnComponentEndOverlap.AddDynamic(this, &AGravitySphere::SetPawnIsOutOfSphere);
	
}

void AGravitySphere::SetPawnIsInSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->SetIsInSphere(true);
	}
}

void AGravitySphere::SetPawnIsOutOfSphere(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if(ABasePawnPlayer* PawnPlayer = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnPlayer->SetIsInSphere(false);
	}
}


