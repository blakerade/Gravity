// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "BulletBase.h"
#include "Components/SphereComponent.h"
#include "Components/TextBlock.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Gravity/Characters/BasePawnPlayer.h"
#include "Gravity/HUD/ShooterHUD.h"
#include "Gravity/HUD/UShooterOverlay.h"
#include "Gravity/PlayerController/GravityPlayerController.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponBodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponBodyMesh);
	WeaponPickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	WeaponPickupSphere->SetupAttachment(WeaponBodyMesh);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	WeaponPickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::ShowPickupWidget);
	WeaponPickupSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponBase::HidePickupWidget);
	Shooter = Cast<ABasePawnPlayer>(GetOwner());
}


void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponBase::ShowPickupWidget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Shooter = Shooter == nullptr ? Cast<ABasePawnPlayer>(OtherActor) : Shooter;
	if(Shooter)
	{
		AGravityPlayerController* ShooterController = Cast<AGravityPlayerController>(Shooter->GetController());
		bool bCanSetVis = ShooterController &&
			ShooterController->ShooterHUD &&
			ShooterController->ShooterHUD->ShooterOverlay &&
			ShooterController->ShooterHUD->ShooterOverlay->PickupText;
		if(bCanSetVis)
		{
			ShooterController->ShooterHUD->ShooterOverlay->PickupText->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void AWeaponBase::HidePickupWidget(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Shooter = Shooter == nullptr ? Cast<ABasePawnPlayer>(OtherActor) : Shooter;
	if(Shooter)
	{
		AGravityPlayerController* ShooterController = Cast<AGravityPlayerController>(Shooter->GetController());
		bool bCanSetVis = ShooterController &&
			ShooterController->ShooterHUD &&
			ShooterController->ShooterHUD->ShooterOverlay &&
			ShooterController->ShooterHUD->ShooterOverlay->PickupText;
		if(bCanSetVis)
		{
			ShooterController->ShooterHUD->ShooterOverlay->PickupText->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AWeaponBase::RequestFire(FVector HitTarget)
{
	Shooter = Shooter == nullptr ? Cast<ABasePawnPlayer>(GetOwner()) : Shooter;
	UWorld* World = GetWorld();
	const USkeletalMeshSocket* Muzzle = WeaponBodyMesh->GetSocketByName(FName("MuzzleFlash"));
	if(Shooter && BulletClass && Muzzle && World)
	{
		ABulletBase* Bullet = World->SpawnActor<ABulletBase>(BulletClass, Muzzle->GetSocketLocation(WeaponBodyMesh), (HitTarget - Muzzle->GetSocketLocation(WeaponBodyMesh)).Rotation());
		if(Bullet)
		{
			Bullet->SetOwner(Shooter);
		}
	}
}


