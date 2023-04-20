// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Gravity/Characters/BasePawnPlayer.h"
#include "Gravity/HUD/ShooterHUD.h"
#include "Gravity/PlayerController/GravityPlayerController.h"
#include "Gravity/Weapons/WeaponBase.h"

UShooterCombatComponent::UShooterCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}


void UShooterCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	Shooter = Cast<ABasePawnPlayer>(GetOwner());
	if(Shooter && Shooter->GetMesh())
	{
		const USkeletalMeshSocket* WeaponSocket = Shooter->GetMesh()->GetSocketByName(FName("RightHandWeaponSlot1"));
		if(WeaponSocket && DefaultWeaponClass && GetWorld())
		{
			const FTransform MeshSlotTransform = WeaponSocket->GetSocketTransform(Shooter->GetMesh());
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = Shooter;
			DefaultWeapon = GetWorld()->SpawnActor<AWeaponBase>(DefaultWeaponClass, MeshSlotTransform);
			WeaponSocket->AttachActor(DefaultWeapon, Shooter->GetMesh());
			EquippedWeapon = DefaultWeapon;
		}
	}
	SetHUDCrossHairs();
}

void UShooterCombatComponent::SetHUDCrossHairs()
{
	PC = PC == nullptr ? Cast<AGravityPlayerController>(GetWorld()->GetFirstPlayerController()) : PC;
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(PC->GetHUD()) : ShooterHUD;
	if(EquippedWeapon && PC && ShooterHUD && ShooterHUD->HUDPackage)
	{
		ShooterHUD->HUDPackage->CrosshairTop = EquippedWeapon->CrosshairTop;
		ShooterHUD->HUDPackage->CrosshairBottom = EquippedWeapon->CrosshairBottom;
		ShooterHUD->HUDPackage->CrosshairRight = EquippedWeapon->CrosshairRight;
		ShooterHUD->HUDPackage->CrosshairLeft = EquippedWeapon->CrosshairLeft;
		ShooterHUD->HUDPackage->CrosshairCenter = EquippedWeapon->CrosshairCenter;
	}
}


void UShooterCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

