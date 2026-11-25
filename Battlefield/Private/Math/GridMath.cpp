#include "Math/GridMath.h"

FVector FGridMath::GridToWorld(int32_t x, int32_t y, float tileSize, FUintVector gridOffset)
{
	return FVector(
		x * tileSize + gridOffset.X,
		y * tileSize + gridOffset.Y,
		0.f);
}

FVector FGridMath::GridToWorld(FGrid::FGridCoordinate coord, float tileSize, FUintVector gridOffset)
{
	return GridToWorld(coord.x, coord.y, tileSize, gridOffset);
}

FVector FGridMath::GridToWorldCentered(int32_t x, int32_t y, float tileSize, uint16_t gridWidth, uint16_t gridHeight,
                                       FUintVector gridOffset)
{
	FVector pos = FVector(
		x * tileSize + gridOffset.X,
		y * tileSize + gridOffset.Y,
		0.f);
	FVector halfSize = FVector(
		gridWidth * tileSize + gridOffset.X,
		gridHeight * tileSize + gridOffset.Y,
		0.f) * 0.5f;
	return pos - halfSize;
}

FVector FGridMath::GridToWorldCentered(
	FGrid::FGridCoordinate coord, float tileSize, uint16_t gridWidth, uint16_t gridHeight, FUintVector gridOffset)
{
	return GridToWorldCentered(coord.x, coord.y, tileSize, gridWidth, gridHeight, gridOffset);
}

FGrid::FGridCoordinate FGridMath::WorldToGrid(const FVector& vector, float tileSize, FUintVector gridOffset)
{
	return FGrid::FGridCoordinate(
		FMath::FloorToInt((vector.X - gridOffset.X) / tileSize),
		FMath::FloorToInt((vector.Y - gridOffset.Y) / tileSize)
	);
}

FGrid::FGridCoordinate FGridMath::WorldCenteredToGrid(const FVector& vector, float tileSize, uint16_t gridWidth,
                                                      uint16_t gridHeight, FUintVector gridOffset)
{
	FVector pos = FVector(
		vector.X - gridOffset.X,
		vector.Y - gridOffset.Y,
		0.f);
	FVector halfSize = FVector(
		gridWidth * tileSize + gridOffset.X,
		gridHeight * tileSize + gridOffset.Y,
		0.f) * 0.5f;
	pos += halfSize;

	return FGrid::FGridCoordinate(
		FMath::FloorToInt(pos.X / tileSize),
		FMath::FloorToInt(pos.Y / tileSize)
	);
}
