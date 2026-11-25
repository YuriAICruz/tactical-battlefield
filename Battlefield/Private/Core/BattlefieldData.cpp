#include "Core/BattlefieldData.h"

void FBattlefieldData::Clear()
{
	mPlayerTroupe.Clear();
	mEnemyTroupe.Clear();
	mNeutralTroupe.Clear();
}

const FTroupe* FBattlefieldData::GetTroupe(ETeams team) const
{
	switch (team)
	{
	case ETeams::Player:
		return &mPlayerTroupe;
	case ETeams::Enemy:
		return &mEnemyTroupe;
	case ETeams::Neutral:
		return &mNeutralTroupe;
	case ETeams::Null:
		break;
	}

	return nullptr;
}

FTroupe* FBattlefieldData::GetTroupe(ETeams team)
{
	switch (team)
	{
	case ETeams::Player:
		return &mPlayerTroupe;
	case ETeams::Enemy:
		return &mEnemyTroupe;
	case ETeams::Neutral:
		return &mNeutralTroupe;
	case ETeams::Null:
		break;
	}

	return nullptr;
}
