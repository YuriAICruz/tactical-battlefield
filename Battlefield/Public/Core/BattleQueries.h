#pragma once
#include "Platform/CoreTypes.h"
#include "Core/Grid.h"
#include "Enums/BattleAction.h"
#include "Enums/Teams.h"

class FBaseUnit;
class FBattlefieldData;
class FTeamSystem;

class BATTLEFIELD_API FBattleQueries
{
public:
	FBattleQueries(const FTeamSystem* teamSystem, const FGrid* grid, const FBattlefieldData* battlefield) :
		mTeamSystem(teamSystem),
		mpGrid(grid),
		mBattlefield(battlefield)
	{
	}

	void GetMovementArea(const FBaseUnit* unit, std::vector<FGrid::FGridCoordinate>& neighbours,
	                     bool passThroughAllies = false) const;
	bool HasWinner() const;
	ETeams GetWinner() const;
	ETeams GetCurrentTeam() const;
	void GetUnitsForTeam(ETeams filterMask, std::vector<FBaseUnit*>& unitsList) const;
	void GetUnitsForCurrentTeam(std::vector<FBaseUnit*>& unitsList) const;
	// Living units of the team that have not waited this phase.
	void GetActableUnits(ETeams team, std::vector<FBaseUnit*>& unitsList) const;
	// True when the team has no living unit left that has not waited (also true for an empty team).
	bool AllUnitsWaited(ETeams team) const;
	FBaseUnit* GetUnitAt(FGrid::FGridCoordinate position) const;
	void GetAttackableTargets(const FBaseUnit* unit, std::vector<const FGridEntity*>& targets) const;
	void GetVisibleUnits(const FBaseUnit* unit, std::vector<const FGridEntity*>& units, ETeams filterMask) const;
	bool HasLineOfSight(const FBaseUnit* unit, FGrid::FGridCoordinate target) const;
	void GetVisionRange(FGrid::FGridCoordinate origin, int range, std::vector<FGrid::FGridCoordinate>& vision,
	                    bool checkBlockedTiles) const;
	void GetNeighbours(FGrid::FGridCoordinate origin, int range,
	                   std::vector<FGrid::FGridCoordinate>& neighbours) const;
	void GetWeightedNeighbours(FGrid::FGridCoordinate origin, int range,
	                           std::vector<FGrid::FGridCoordinate>& neighbours, bool checkBlockedTiles) const;
	FBaseUnit* FindNearestUnit(const FBaseUnit* unit, int32 range) const;
	void GetPath(FGrid::FGridCoordinate origin, FGrid::FGridCoordinate destination, int32 size,
	             std::vector<FGrid::FGridCoordinate>& path) const;
	void GetPathTo(const FBaseUnit* unit, const FBaseUnit* target, int32 size,
	               std::vector<FGrid::FGridCoordinate>& path) const;
	int32 GetUnitMovementCost(const FBaseUnit* unit, FGrid::FGridCoordinate destination) const;
	static EBattleAction GetValidActions(const FBaseUnit* unit, int32 cost = 1);

private:
	const FTeamSystem* mTeamSystem;
	const FGrid* mpGrid;
	const FBattlefieldData* mBattlefield;
};
