#include "Core/GridEntity.h"

uint32 FGridEntity::GetUnitId() const
{
	return mUnitId;
}

bool FGridEntity::CanMove(int32 cost) const
{
	return true;
}

ETeams FGridEntity::GetTeam() const
{
	return mTeam;
}

void FGridEntity::SetTeam(ETeams team)
{
	mTeam = team;
}