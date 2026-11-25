#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"
#include "Core/Grid.h"

class FBattleMoveEvent : public FBaseEvent
{
public:
	FBattleMoveEvent(FGrid::FGridCoordinate to) : To(to)
	{
	}

	FGrid::FGridCoordinate To;
};
