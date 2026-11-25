#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"

class FBaseUnit;

class FBattleHealedEvent : public FBaseEvent
{
public:
	FBattleHealedEvent(const FBaseUnit* target, int32 value) :
		Target(target),
		Value(value)
	{
	}

	const FBaseUnit* Target;
	int32 Value;
};
