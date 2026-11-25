#pragma once
#include "Platform/CoreTypes.h"
#include "Enums/Teams.h"

class BATTLEFIELD_API FGridEntity
{
public:
	FGridEntity() : mUnitId(0)
	{
	}

	FGridEntity(uint32 unitId) : mUnitId(unitId)
	{
	}

	virtual ~FGridEntity() = default;
	virtual bool CanMove(int32 cost = 1) const;

	virtual uint32 GetUnitId() const;

	ETeams GetTeam() const;
	void SetTeam(ETeams team);

	bool operator==(const FGridEntity& Other) const
	{
		return this == &Other;
	}

	bool operator!=(const FGridEntity& Other) const
	{
		return !(*this == Other);
	}

private:
	ETeams mTeam = static_cast<ETeams>(0);
	const uint32 mUnitId = 0;
};
