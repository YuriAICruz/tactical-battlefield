#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"

class FBaseUnit;

class FBattleAttackEvent : public FBaseEvent
{
public:
	FBattleAttackEvent(const FBaseUnit* attacker, const FBaseUnit* target, int32 damage) :
		Attacker(attacker),
		Target(target),
		Damage(damage)
	{
	}

	const FBaseUnit* Attacker;
	const FBaseUnit* Target;
	int32 Damage;
};
