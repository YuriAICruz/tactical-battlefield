#pragma once
#include "Platform/CoreTypes.h"
#include "BaseUnit.h"

class FBaseUnit;

class TeamSystemHelper
{
public:
	static bool IsHostile(const FBaseUnit* A, const FBaseUnit* B)
	{
		if(A->GetTeam() == ETeams::Null || B->GetTeam() == ETeams::Null)
		{
			return false;
		}

		//TODO: Neutral still return hostile
		
		return A->GetTeam() != B->GetTeam();
	}
	
};
