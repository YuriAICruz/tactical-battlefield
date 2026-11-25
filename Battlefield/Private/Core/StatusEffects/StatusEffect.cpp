#include "Core/StatusEffects/StatusEffect.h"
#include "Core/BaseUnit.h"

void FStatusEffect::AddTurnTick(FBaseUnit* unit)
{
	FBaseStatusEffect::AddTurnTick(unit);

	unit->SpendActionPoints(unit->GetMaxActionPoints());
	unit->SpendMovementPoints(unit->GetMaxMovementPoints());
}
