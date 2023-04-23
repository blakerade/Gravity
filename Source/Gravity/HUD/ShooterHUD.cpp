// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHUD.h"

#include "UShooterOverlay.h"
#include "Blueprint/UserWidget.h"

void AShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	if(GEngine && GEngine->GameViewport)
	{
		FVector2d ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		HUDPackage.CrosshairColor = FLinearColor::White;
		HUDPackage.CrosshairSpread = 10.f;
		if(HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshairs(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -HUDPackage.CrosshairSpread);
			DrawCrosshairs(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, HUDPackage.CrosshairSpread);
			DrawCrosshairs(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairRight)
		{
			FVector2D Spread(HUDPackage.CrosshairSpread, 0.f);
			DrawCrosshairs(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-HUDPackage.CrosshairSpread, 0.f);
			DrawCrosshairs(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
	
}

void AShooterHUD::DrawCrosshairs(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D CrosshairSpread, FLinearColor TextureColor)
{
	float TextureHeight = Texture->GetSizeY();
	float TextureWidth = Texture->GetSizeX();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f) + CrosshairSpread.X, ViewportCenter.Y - (TextureHeight / 2.f) + CrosshairSpread.Y);
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, TextureColor);
}

void AShooterHUD::AddShooterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if(PC && ShooterOverlayClass)
	{
		ShooterOverlay = CreateWidget<UShooterOverlay>(PC,ShooterOverlayClass);
		if(ShooterOverlay)
		{
			ShooterOverlay->AddToViewport();
		}
	}
}
