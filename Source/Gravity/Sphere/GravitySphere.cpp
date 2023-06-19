// Fill out your copyright notice in the Description page of Project Settings.


#include "GravitySphere.h"

#include "Components/SphereComponent.h"
#include "Gravity/Characters/BasePawnPlayer.h"


AGravitySphere::AGravitySphere()
{
	PrimaryActorTick.bCanEverTick = true;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	
}

void AGravitySphere::BeginPlay()
{
	Super::BeginPlay();
	
	if(OverlapSphere)
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &AGravitySphere::AddSphereLevel);
	}
	BeginPlayAddSphereLevel();
}

void AGravitySphere::BeginPlayAddSphereLevel()
{
	TArray<FOverlapResult> OverlapResults;
	const FCollisionShape GravitySphere = FCollisionShape::MakeSphere(OverlapSphere->GetScaledSphereRadius());
	GetWorld()->OverlapMultiByChannel(OverlapResults, GetActorLocation(), FQuat::Identity, ECollisionChannel::ECC_Pawn, GravitySphere);
	for(FOverlapResult PawnResult: OverlapResults)
	{
		if(ABasePawnPlayer* PawnToSet = Cast<ABasePawnPlayer>(PawnResult.GetActor()))
		{
			PawnToSet->SetLevelSphere(this);
		}
	}
}

void AGravitySphere::AddSphereLevel(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if(ABasePawnPlayer* PawnToSet = Cast<ABasePawnPlayer>(OtherActor))
	{
		PawnToSet->SetLevelSphere(this);
	}
}



