#pragma once
#include "Platform/CoreTypes.h"
#include "Grid.h"
#include "TeamSystem.h"
#include "Troupe.h"

class BATTLEFIELD_API FBattlefieldData
{
public:
	FBattlefieldData(FTeamSystem* teamSystem) :
		mPlayerTroupe(teamSystem),
		mEnemyTroupe(teamSystem),
		mNeutralTroupe(teamSystem)
	{
	}

	void Clear();
	const FTroupe* GetTroupe(ETeams team) const;
	FTroupe* GetTroupe(ETeams team);

private:
	FTroupe mPlayerTroupe;
	FTroupe mEnemyTroupe;
	FTroupe mNeutralTroupe;
};
