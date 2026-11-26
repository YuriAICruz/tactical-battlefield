#pragma once
#include "Platform/CoreTypes.h"
#include "BaseUnit.h"
#include "Enums/Teams.h"

class BATTLEFIELD_API FTeamSystem
{
public:
	FTeamSystem();
	ETeams GetCurrentTeam() const;
	void AddActor(const FBaseUnit* actor);
	void RemoveActor(const FBaseUnit* actor);
	void EndTurn();
	bool CanAct(const FBaseUnit* actor, bool checkActor = true) const;
	bool IsTeamDefeated(ETeams team) const;
	bool HasWinner() const;
	ETeams GetWinner() const;
	uint32 GetNextId() const;
	// When true, CanAct rejects units that have waited this phase. Default false.
	void SetRequireWait(bool require);
	bool GetRequireWait() const;

private:
	bool mRequireWait = false;
	ETeams mCurrentTeam;
	uint32 mCurrentTeamsUnitsCount;
	std::vector<const FBaseUnit*> mActors;
};
