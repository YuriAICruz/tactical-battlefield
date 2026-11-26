#include "Core/TeamSystem.h"

ETeams GetNextTeam(ETeams team)
{
	switch (team)
	{
	case ETeams::Player:
		return ETeams::Enemy;

	case ETeams::Enemy:
		return ETeams::Neutral;

	case ETeams::Neutral:
		return ETeams::Player;

	default:
		return ETeams::Player;
	}
}

FTeamSystem::FTeamSystem()
	: mCurrentTeam(ETeams::Player), mCurrentTeamsUnitsCount(0)
{
}

ETeams FTeamSystem::GetCurrentTeam() const
{
	return mCurrentTeam;
}

void FTeamSystem::AddActor(const FBaseUnit* actor)
{
	if (actor == nullptr)
	{
		return;
	}

	mActors.push_back(actor);
	mCurrentTeamsUnitsCount++;
}

void FTeamSystem::RemoveActor(const FBaseUnit* actor)
{
	std::erase(mActors, actor);
}

void FTeamSystem::EndTurn()
{
	mCurrentTeam = GetNextTeam(mCurrentTeam);

	for (const FBaseUnit* actor : mActors)
	{
		if (actor == nullptr)
		{
			continue;
		}

		if (!actor->IsAlive())
		{
			continue;
		}

		if (actor->GetTeam() != mCurrentTeam)
		{
			continue;
		}

		const_cast<FBaseUnit*>(actor)->ResetTurnStats();
	}
}

bool FTeamSystem::CanAct(const FBaseUnit* actor, bool checkActor) const
{
	if (actor == nullptr)
	{
		return false;
	}

	if (checkActor)
	{
		if (mRequireWait && actor->HasWaited())
		{
			return false;
		}

		if (!actor->CanMove() && !actor->CanDoAction())
		{
			return false;
		}
	}

	return actor->GetTeam() == mCurrentTeam;
}

bool FTeamSystem::IsTeamDefeated(ETeams team) const
{
	bool bFoundLivingActor = false;

	for (const FBaseUnit* actor : mActors)
	{
		if (actor == nullptr)
		{
			continue;
		}

		if (actor->GetTeam() != team)
		{
			continue;
		}

		if (actor->IsAlive())
		{
			bFoundLivingActor = true;
			break;
		}
	}

	return !bFoundLivingActor;
}

bool FTeamSystem::HasWinner() const
{
	const bool bPlayerDefeated = IsTeamDefeated(ETeams::Player);
	const bool bEnemyDefeated = IsTeamDefeated(ETeams::Enemy);

	return bPlayerDefeated || bEnemyDefeated;
}

ETeams FTeamSystem::GetWinner() const
{
	const bool bPlayerDefeated = IsTeamDefeated(ETeams::Player);
	const bool bEnemyDefeated = IsTeamDefeated(ETeams::Enemy);

	if (bPlayerDefeated && bEnemyDefeated)
	{
		// Mutual destruction: nobody won
		return ETeams::Null;
	}

	if (bEnemyDefeated)
	{
		return ETeams::Player;
	}

	// Enemy wins by default
	return ETeams::Enemy;
}

uint32 FTeamSystem::GetNextId() const
{
	return mCurrentTeamsUnitsCount;
}

void FTeamSystem::SetRequireWait(bool require)
{
	mRequireWait = require;
}

bool FTeamSystem::GetRequireWait() const
{
	return mRequireWait;
}
