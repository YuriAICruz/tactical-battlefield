#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"

class FBaseUnit;

class FBattleHealEvent : public FBaseEvent
{
public:
	FBattleHealEvent(const FBaseUnit* attacker, const FBaseUnit* target, int32 value) :
		Attacker(attacker),
		Target(target),
		Value(value)
	{
	}

	const FBaseUnit* Attacker;
	const FBaseUnit* Target;
	int32 Value;
};

// Deprecated alias kept for one release; use FBattleHealEvent.
using BattleHealEvent = FBattleHealEvent;
