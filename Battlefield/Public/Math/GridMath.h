#pragma once
#include "Platform/CoreTypes.h"
#include "Core/Grid.h"
#include "Math/Vector.h"

// Grid <-> world space conversions (pure C++ port of the Unreal FGridMath helper).
// The Blueprint conversion helpers (FGridCoord / TArray) only exist in the Unreal wrapper.
class BATTLEFIELD_API FGridMath
{
public:
	static FVector GridToWorld(int32_t x, int32_t y, float tileSize, FUintVector gridOffset);
	static FVector GridToWorld(FGrid::FGridCoordinate coord, float tileSize, FUintVector gridOffset);
	static FVector GridToWorldCentered(int32_t x, int32_t y, float tileSize, uint16_t gridWidth, uint16_t gridHeight,
	                                   FUintVector gridOffset);
	static FVector GridToWorldCentered(FGrid::FGridCoordinate coord, float tileSize, uint16_t gridWidth,
	                                   uint16_t gridHeight, FUintVector gridOffset);
	static FGrid::FGridCoordinate WorldToGrid(const FVector& vector, float tileSize, FUintVector gridOffset);
	static FGrid::FGridCoordinate WorldCenteredToGrid(const FVector& vector, float tileSize, uint16_t gridWidth,
	                                                  uint16_t gridHeight, FUintVector gridOffset);
};
