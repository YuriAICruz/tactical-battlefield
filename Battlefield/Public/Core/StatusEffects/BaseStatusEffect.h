#pragma once
#include "Platform/CoreTypes.h"

class FBaseUnit;

class BATTLEFIELD_API FBaseStatusEffect
{
public:	
	FBaseStatusEffect(FBaseUnit* target) : mpUnit(target) {  }
	virtual ~FBaseStatusEffect() = default;
	
	virtual void AddTurnTick(FBaseUnit* unit);
	virtual int32 GetTicksApplied() const;
	
	virtual const uint8& GetType() const = 0;

	FBaseUnit* GetUnit() const;

protected:
	FBaseUnit* mpUnit;
	uint32 mTicks = 0;
};
