#pragma once
#include "Platform/CoreTypes.h"
#include "BaseEvent.h"

class FBaseUnit;

class FBattleAttackEvent : public FBaseEvent
{
public:
	FBattleAttackEvent(const FBaseUnit* attacker, const FBaseUnit* target, int32 damage, bool hit = true,
	                   bool critical = false, int32 userValue = 0) :
		Attacker(attacker),
		Target(target),
		Damage(damage),
		Hit(hit),
		Critical(critical),
		UserValue(userValue)
	{
	}

	const FBaseUnit* Attacker;
	const FBaseUnit* Target;
	int32 Damage; // damage actually requested (0 on a miss)
	bool Hit; // false on a miss: show "Miss"
	bool Critical;
	int32 UserValue; // opaque, copied from FAttackResult
};
