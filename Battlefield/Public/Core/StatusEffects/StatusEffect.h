#pragma once
#include "Platform/CoreTypes.h"
#include "Core/StatusEffects/BaseStatusEffect.h"

class BATTLEFIELD_API FStatusEffect : public FBaseStatusEffect
{
public:
	virtual const uint8& GetType() const override
	{
		return mType;
	}

	virtual void AddTurnTick(FBaseUnit* unit) override;

	FStatusEffect(FBaseUnit* target, uint8 type) : FBaseStatusEffect(target), mType(type)
	{
	}

private:
	uint8 mType;
};
