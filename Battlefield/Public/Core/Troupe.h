#pragma once
#include "Platform/CoreTypes.h"

#include "Enums/Teams.h"
#include "Core/TeamSystem.h"
#include <memory>

class FBaseUnit;
class FTeamSystem;

class BATTLEFIELD_API FTroupe
{
public:
	FTroupe();
	FTroupe(FTeamSystem* teamSystem);
	
	FTroupe(const FTroupe&) = delete;
	FTroupe& operator=(const FTroupe&) = delete;

	FTroupe(FTroupe&&) = default;
	FTroupe& operator=(FTroupe&&) = default;

	template<typename T>
	requires std::derived_from<T, FBaseUnit>
	FBaseUnit* CreateActor(const FBaseUnit::FCombatStats& combatStats, const FBaseUnit::FTurnResources& turnResources, const FBaseUnit::FCombatWeapon& weapon, ETeams Team)
	{
		auto Actor = std::make_unique<T>(mpTeamSystem->GetNextId(), combatStats, turnResources, weapon, Team);
		FBaseUnit* Result = Actor.get();

		mUnits.push_back(std::move(Actor));
		mpTeamSystem->AddActor(Result);

		return Result;
	}
	void DestroyActor(FBaseUnit* unit);

	void GetLiveUnits(std::vector<FBaseUnit*>& unitList, bool clear = true) const;
	void GetUnits(std::vector<const FBaseUnit*>& unitList, bool clear = true) const;
	void Clear();

private:
	FTeamSystem* mpTeamSystem;
	std::vector<std::unique_ptr<FBaseUnit>> mUnits;
};
