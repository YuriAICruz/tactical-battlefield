#pragma once
#include "Platform/CoreTypes.h"

// Outcome of one attack, decided by the client through FBattleController::OnResolveAttack.
// The DLL applies it as given; it does not know what "hit chance" or "critical" mean.
struct FAttackResult
{
	int32 damage = 0;
	bool hit = true; // false: the attack is a miss, damage is ignored (treated as 0)
	bool critical = false; // informational, forwarded in the event
	int32 userValue = 0; // opaque, forwarded in the event
};
