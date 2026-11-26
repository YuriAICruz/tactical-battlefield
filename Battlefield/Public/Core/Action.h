#pragma once
#include "Platform/CoreTypes.h"
#include "Core/Grid.h"

class FBaseUnit;

// A client-defined action (Attack, Wait, Heal, Rescue, Items...). The DLL only validates team and AP, calls the
// callbacks, spends the AP and applies "ends the unit's turn". It has no idea what the action does.
struct BATTLEFIELD_API FActionDef
{
	FName id;
	FName name;
	int32 apCost = 1;
	bool endsUnitTurn = false; // after a successful execute the unit waits (feature 04)

	// Optional. Unset means "always usable".
	std::function<bool(const FBaseUnit&)> canUse;
	// Optional, for UIs and AI: tiles the action can be aimed at. The DLL never calls it itself.
	std::function<void(const FBaseUnit&, std::vector<FGrid::FGridCoordinate>& out)> getTargets;
	// Optional. Unset means "succeeds with no effect" (a pure cost / end-turn action such as Wait).
	// Returning false cancels the action: nothing is spent and no event fires.
	std::function<bool(FBaseUnit&, FGrid::FGridCoordinate target)> execute;
};

// Id the legacy FBattleController::DoUnitAction delegates to when a client registers it.
inline const FName kDefaultActionId = "default";
