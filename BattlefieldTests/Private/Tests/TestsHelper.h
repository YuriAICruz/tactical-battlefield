#pragma once
#include <algorithm>
#include "Core/BaseUnit.h"
#include "Core/BattleController.h"

class TestsHelper
{
public:
	static constexpr FBaseUnit::FCombatStats kBaseCombatStats
	{
		10,
		10,
		0,
		10,
		0
	};

	static constexpr FBaseUnit::FTurnResources kBaseTurnResources
	{
		5,
		6
	};

	static constexpr FBaseUnit::FCombatWeapon kCombatWeapon
	{
		5,
		1,
		5
	};

	static FBattleController CreateController(FGrid* grid)
	{
		return FBattleController(grid);
	}

	static inline bool ContainsTile(const std::vector<FGrid::FGridCoordinate>& Tiles, const FGrid::FGridCoordinate& Coord)
	{
		return std::ranges::find(Tiles, Coord) != Tiles.end();
	}

	static FGrid CreateTestGrid(int width = 10, int height = 10)
	{
		return FGrid(static_cast<uint16_t>(width), static_cast<uint16_t>(height));
	}
	
};