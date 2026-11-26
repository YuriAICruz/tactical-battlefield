#include "Core/BattleQueries.h"
#include "Core/BaseUnit.h"
#include "Core/BattlefieldData.h"
#include "Core/TeamSystem.h"
#include "Core/Troupe.h"

static inline bool ContainsTile(const std::vector<FGrid::FGridCoordinate>& Tiles, const FGrid::FGridCoordinate& Coord)
{
	return std::ranges::find(Tiles, Coord) != Tiles.end();
}

void FBattleQueries::GetMovementArea(const FBaseUnit* unit, std::vector<FGrid::FGridCoordinate>& neighbours,
                                     bool passThroughAllies) const
{
	const FGrid::FGridCoordinate pos = mpGrid->GetUnitPosition(unit);
	int32 size = unit->GetMovementPoints();
	mpGrid->GetWeightedNeighbours(pos, neighbours, size, true, passThroughAllies ? unit->GetTeam() : ETeams::Null);
}

bool FBattleQueries::HasWinner() const
{
	return mTeamSystem->HasWinner();
}

ETeams FBattleQueries::GetWinner() const
{
	return mTeamSystem->GetWinner();
}

ETeams FBattleQueries::GetCurrentTeam() const
{
	return mTeamSystem->GetCurrentTeam();
}

void FBattleQueries::GetUnitsForTeam(ETeams filterMask, std::vector<FBaseUnit*>& unitsList) const
{
	unitsList.clear();

	constexpr ETeams Teams[]
	{
		ETeams::Player,
		ETeams::Enemy,
		ETeams::Neutral
	};

	for (ETeams team : Teams)
	{
		if (!HasFlag(filterMask, team))
		{
			continue;
		}

		const FTroupe* troupe = mBattlefield->GetTroupe(team);
		if (!troupe)
		{
			continue;
		}

		troupe->GetLiveUnits(unitsList, false);
	}
}

void FBattleQueries::GetUnitsForCurrentTeam(std::vector<FBaseUnit*>& unitsList) const
{
	GetUnitsForTeam(GetCurrentTeam(), unitsList);
}

void FBattleQueries::GetActableUnits(ETeams team, std::vector<FBaseUnit*>& unitsList) const
{
	GetUnitsForTeam(team, unitsList);
	std::erase_if(unitsList, [](const FBaseUnit* unit) { return unit->HasWaited(); });
}

bool FBattleQueries::AllUnitsWaited(ETeams team) const
{
	std::vector<FBaseUnit*> actable;
	GetActableUnits(team, actable);
	return actable.empty();
}

FBaseUnit* FBattleQueries::GetUnitAt(FGrid::FGridCoordinate position) const
{
	const FGrid::FTileData* tile = mpGrid->TryGetTile(position);
	if (!tile)
	{
		return nullptr;
	}
	return static_cast<FBaseUnit*>(tile->unit);
}

void FBattleQueries::GetAttackableTargets(
	const FBaseUnit* unit, std::vector<const FGridEntity*>& targets) const
{
	GetAttackableTargetsFrom(unit, mpGrid->GetUnitPosition(unit), targets);
}

void FBattleQueries::GetAttackArea(const FBaseUnit* unit, FGrid::FGridCoordinate fromTile,
                                   std::vector<FGrid::FGridCoordinate>& area) const
{
	area.clear();
	const FWeaponDef* weapon = unit ? unit->GetEquippedWeapon() : nullptr;
	if (!weapon || weapon->maxRange <= 0)
	{
		return;
	}

	std::vector<FGrid::FGridCoordinate> inReach;
	mpGrid->GetNeighbours(fromTile, inReach, static_cast<uint16_t>(weapon->maxRange));
	for (const FGrid::FGridCoordinate& coord : inReach)
	{
		const int32 distance = std::abs(coord.x - fromTile.x) + std::abs(coord.y - fromTile.y);
		if (weapon->IsInRange(distance))
		{
			area.push_back(coord);
		}
	}
}

void FBattleQueries::GetAttackableTargetsFrom(const FBaseUnit* unit, FGrid::FGridCoordinate tile,
                                              std::vector<const FGridEntity*>& targets) const
{
	targets.clear();
	if (!unit)
	{
		return;
	}

	std::vector<FGrid::FGridCoordinate> area;
	GetAttackArea(unit, tile, area);
	mpGrid->GetUnitsOnArea(area, targets, Invert(unit->GetTeam()));
}

void FBattleQueries::GetVisibleUnits(const FBaseUnit* unit, std::vector<const FGridEntity*>& units,
                                     ETeams filterMask) const
{
	units.clear();

	if (!unit)
	{
		return;
	}

	const FGrid::FGridCoordinate origin = mpGrid->GetUnitPosition(unit);
	const int32 visionRange = unit->GetVisionRange();

	std::vector<FBaseUnit*> otherUnits;
	GetUnitsForTeam(filterMask, otherUnits);

	units.reserve(otherUnits.size());

	for (const FBaseUnit* candidate : otherUnits)
	{
		if (!candidate->IsAlive())
		{
			continue;
		}

		const FGrid::FGridCoordinate target =
			mpGrid->GetUnitPosition(candidate);

		if (mpGrid->GetDistance(origin, target) > visionRange)
		{
			continue;
		}

		if (!mpGrid->HasLineOfSight(origin, target, false))
		{
			continue;
		}

		units.push_back(candidate);
	}


	int32 range = unit->GetVisionRange();
	std::vector<FGrid::FGridCoordinate> area;
	mpGrid->GetWeightedNeighbours(mpGrid->GetUnitPosition(unit), area, range, false);

	ETeams team = unit->GetTeam();
	mpGrid->GetUnitsOnArea(area, units, Invert(team));
}

bool FBattleQueries::HasLineOfSight(const FBaseUnit* unit, FGrid::FGridCoordinate target) const
{
	if (!unit)
	{
		return false;
	}
	const FGrid::FGridCoordinate origin = mpGrid->GetUnitPosition(unit);

	if (mpGrid->GetDistance(origin, target) > unit->GetVisionRange())
	{
		return false;
	}
	return mpGrid->HasLineOfSight(origin, target, false);
}

void FBattleQueries::GetVisionRange(
	FGrid::FGridCoordinate origin, int range, std::vector<FGrid::FGridCoordinate>& vision, bool checkBlockedTiles) const
{
	mpGrid->GetVisibleTiles(origin, range, vision, checkBlockedTiles);
}

void FBattleQueries::GetNeighbours(
	FGrid::FGridCoordinate origin, int range, std::vector<FGrid::FGridCoordinate>& neighbours) const
{
	mpGrid->GetNeighbours(origin, neighbours, range);
}

void FBattleQueries::GetWeightedNeighbours(
	FGrid::FGridCoordinate origin, int range, std::vector<FGrid::FGridCoordinate>& neighbours,
	bool checkBlockedTiles) const
{
	mpGrid->GetWeightedNeighbours(origin, neighbours, range, checkBlockedTiles);
}

FBaseUnit* FBattleQueries::FindNearestUnit(const FBaseUnit* unit, int32 range) const
{
	if (unit == nullptr)
	{
		return nullptr;
	}

	const FGrid::FGridCoordinate origin = mpGrid->GetUnitPosition(unit);
	std::vector<FGrid::FGridCoordinate> neighbours;
	mpGrid->GetNeighbours(origin, neighbours, range);
	std::vector<const FGridEntity*> units;
	mpGrid->GetUnitsOnArea(neighbours, units, Invert(unit->GetTeam()));

	FBaseUnit* nearestUnit = nullptr;
	int32 nearestDistance = INT32_MAX;

	for (const FGridEntity* entity : units)
	{
		const FBaseUnit* candidate = static_cast<const FBaseUnit*>(entity);

		if (!candidate->IsAlive())
		{
			continue;
		}

		const FGrid::FGridCoordinate candidatePos = mpGrid->GetUnitPosition(candidate);

		const int32 distance = mpGrid->GetWeightedDistance(origin, candidatePos, false);

		if (distance < 0)
		{
			continue;
		}

		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestUnit = const_cast<FBaseUnit*>(candidate);
		}
	}

	return nearestUnit;
}

void FBattleQueries::GetPath(FGrid::FGridCoordinate origin, FGrid::FGridCoordinate destination, int32 size,
                             std::vector<FGrid::FGridCoordinate>& path) const
{
	mpGrid->GetWeightedPath(origin, destination, path, size, false);
}

void FBattleQueries::GetPathTo(const FBaseUnit* unit, const FBaseUnit* target, int32 size,
                               std::vector<FGrid::FGridCoordinate>& path) const
{
	const FGrid::FGridCoordinate origin = mpGrid->GetUnitPosition(unit);
	const FGrid::FGridCoordinate destination = mpGrid->GetUnitPosition(target);

	GetPath(origin, destination, size, path);
}

int32 FBattleQueries::GetUnitMovementCost(const FBaseUnit* unit, FGrid::FGridCoordinate destination) const
{
	return unit->GetMovementCost(mpGrid, destination);
}

EBattleAction FBattleQueries::GetValidActions(const FBaseUnit* unit, int32 cost)
{
	EBattleAction actions = EBattleAction::None;

	if (!unit->IsAlive())
	{
		return actions;
	}

	if (unit->CanMove(cost))
	{
		ActionAddFlag(actions, EBattleAction::Move);
	}
	if (unit->CanAttack(cost))
	{
		ActionAddFlag(actions, EBattleAction::Attack);
	}
	if (unit->CanDoAction(cost))
	{
		ActionAddFlag(actions, EBattleAction::DoActions);
	}

	return actions;
}
