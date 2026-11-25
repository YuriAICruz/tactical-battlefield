#include "Core/StatusEffects/BaseStatusEffect.h"

void FBaseStatusEffect::AddTurnTick(FBaseUnit* unit)
{
	mTicks++;
}

int32 FBaseStatusEffect::GetTicksApplied() const
{
	return mTicks;
}

FBaseUnit* FBaseStatusEffect::GetUnit() const
{
	return mpUnit;
}
