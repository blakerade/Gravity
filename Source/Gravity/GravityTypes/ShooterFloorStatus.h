#pragma once

UENUM()
enum class EShooterFloorStatus : uint8
{
	NoFloorContact UMETA(DisplayName = "No Floor Contact"),
	BaseFloorContact UMETA(DisplayName = "Base Floor Contact"),
	SphereFloorContact UMETA(DisplayName = "Sphere Floor Contact"),
	SphereLevelContact UMETA(DisplayName = "Sphere Level Contact"),
};
