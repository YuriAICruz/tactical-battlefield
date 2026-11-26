#pragma once
#include "Platform/CoreTypes.h"

// Data-driven weapon. The DLL only stores range, cost, damage and an opaque property map; it never
// interprets what a weapon "is" (sword, lance, bow...). All of that belongs to the client.
struct BATTLEFIELD_API FWeaponDef
{
	FName id;
	FName name;
	int32 minRange = 1; // inclusive, Manhattan distance
	int32 maxRange = 1; // inclusive, Manhattan distance
	int32 baseDamage = 0;
	int32 apCost = 1;
	int32 typeTag = 0; // opaque, never interpreted
	std::map<FName, int32> properties; // opaque (might, hit, weight, durability...)

	bool IsInRange(int32 distance) const
	{
		return distance >= minRange && distance <= maxRange;
	}
};
