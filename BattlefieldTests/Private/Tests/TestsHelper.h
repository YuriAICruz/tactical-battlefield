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

	static FWeaponDef MakeWeapon(const FName& id, int32 minRange, int32 maxRange, int32 baseDamage = 3, int32 apCost = 1)
	{
		FWeaponDef weapon;
		weapon.id = id;
		weapon.name = id;
		weapon.minRange = minRange;
		weapon.maxRange = maxRange;
		weapon.baseDamage = baseDamage;
		weapon.apCost = apCost;
		return weapon;
	}

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

	static FBaseUnit* SpawnUnit(FBattleController& controller, ETeams team, FGrid::FGridCoordinate position)
	{
		return controller.SpawnUnit(team, kBaseCombatStats, kBaseTurnResources, kCombatWeapon, position);
	}

	// 10x10 grid with a wall at x = 4 that only leaves (4, 3) open: the only way across is through that tile.
	static FGrid CreateCorridorGrid()
	{
		FGrid grid = CreateTestGrid();
		for (int y = 0; y < 10; ++y)
		{
			if (y != 3)
			{
				grid.SetTileWeight(4, y, FGrid::kImpassableWeight);
			}
		}
		return grid;
	}
};