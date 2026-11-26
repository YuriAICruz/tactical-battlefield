#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"
#include "Core/Grid.h"

class FBaseUnit;

class FBattleMoveEvent : public FBaseEvent
{
public:
	FBattleMoveEvent(const FBaseUnit* unit, FGrid::FGridCoordinate from, FGrid::FGridCoordinate to) :
		Unit(unit),
		From(from),
		To(to)
	{
	}

	const FBaseUnit* Unit;
	FGrid::FGridCoordinate From;
	FGrid::FGridCoordinate To;
};
