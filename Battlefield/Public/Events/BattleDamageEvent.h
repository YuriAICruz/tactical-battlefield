#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"

class FBaseUnit;

class FBattleDamageEvent : public FBaseEvent
{
public:
	FBattleDamageEvent(const FBaseUnit* target, int32 damage) :
		Target(target),
		Damage(damage)
	{
	}

	const FBaseUnit* Target;
	int32 Damage;
};
