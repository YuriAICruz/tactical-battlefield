#include "Core/Troupe.h"
#include "Core/TeamSystem.h"

FTroupe::FTroupe(): mpTeamSystem(nullptr)
{
}

FTroupe::FTroupe(FTeamSystem* teamSystem) :
	mpTeamSystem(teamSystem)
{
}

void FTroupe::DestroyActor(FBaseUnit* unit)
{
	if (unit == nullptr)
	{
		return;
	}

	if (mpTeamSystem != nullptr)
	{
		mpTeamSystem->RemoveActor(unit);
	}

	std::erase_if(
		mUnits,
		[unit](const std::unique_ptr<FBaseUnit>& current)
		{
			return current.get() == unit;
		}
	);
}

void FTroupe::GetLiveUnits(std::vector<FBaseUnit*>& unitList, bool clear) const
{
	if (clear)
	{
		unitList.clear();
		unitList.reserve(mUnits.size());
	}
	for (const std::unique_ptr<FBaseUnit>& unit : mUnits)
	{
		if (unit->IsAlive())
		{
			unitList.push_back(unit.get());
		}
	}
}

void FTroupe::GetUnits(std::vector<const FBaseUnit*>& unitList, bool clear) const
{
	if (clear)
	{
		unitList.clear();
		unitList.reserve(mUnits.size());
	}
	for (const std::unique_ptr<FBaseUnit>& unit : mUnits)
	{
		unitList.push_back(unit.get());
	}
}

void FTroupe::Clear()
{
	if (mpTeamSystem != nullptr)
	{
		for (const std::unique_ptr<FBaseUnit>& unit : mUnits)
		{
			mpTeamSystem->RemoveActor(unit.get());
		}
	}

	int32 size = static_cast<int32>(mUnits.size());
	mUnits.clear();
	mUnits.reserve(size);
}
