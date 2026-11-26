#include "Core/BaseUnit.h"
#include "Core/TeamSystemHelper.h"
#include "Core/StatusEffects/BaseStatusEffect.h"
#include "Core/StatusEffects/StatusEffect.h"

FBaseUnit::FBaseUnit(const uint32& unitId) : FGridEntity(unitId)
{
}

bool FBaseUnit::IsEnemy(const FBaseUnit* actor) const
{
	return TeamSystemHelper::IsHostile(this, actor);
}

bool FBaseUnit::IsAlive() const
{
	return mBaseCombatStats.health > 0;
}

void FBaseUnit::ReceiveDamage(int32 damage)
{
	if (damage <= 0)
	{
		return;
	}

	if (!IsAlive())
	{
		return;
	}
	mBaseCombatStats.health = std::max(mBaseCombatStats.health - damage, 0);
	if (OnReceiveDamage)
	{
		OnReceiveDamage(this, damage);
	}
	if (!IsAlive())
	{
		if (OnDeath) OnDeath(this);
	}
}

void FBaseUnit::ReceiveHeal(int32 heal)
{
	if (heal <= 0)
	{
		return;
	}

	if (!IsAlive())
	{
		return;
	}

	mBaseCombatStats.health = std::min(mBaseCombatStats.health + heal, GetMaxHealth());
	if (OnReceivedHeal)
	{
		OnReceivedHeal(this, heal);
	}
}

int32 FBaseUnit::CalculateWeaponAttackDamage(FGrid::FGridCoordinate position, FGrid::FGridCoordinate targetPosition) const
{
	// TODO: Create a BP way to do this calculation on the weapon
	int32 distance = GetDistance(position, targetPosition);
	float mul = std::min(1, mBaseCombatStats.aim / distance);
	return mCurrentCombatWeapon.baseDamage * mul;
}

bool FBaseUnit::InRange(FGrid::FGridCoordinate position, FGrid::FGridCoordinate targetPosition) const
{
	int32 distance = GetDistance(position, targetPosition);
	return distance <= mCurrentCombatWeapon.range;
}

int32 FBaseUnit::GetMovementCost(const FGrid* grid, FGrid::FGridCoordinate destination, bool passThroughAllies) const
{
	FGrid::FGridCoordinate start = grid->GetUnitPosition(this);
	return grid->GetWeightedDistance(start, destination, true, passThroughAllies ? GetTeam() : ETeams::Null);
}

bool FBaseUnit::CanMove(int32 cost) const
{
	if (mCurrentTurnResources.move < cost)
	{
		return false;
	}

	return IsAlive();
}

bool FBaseUnit::CanDoAction(int32 cost) const
{
	if (mCurrentTurnResources.ap < cost)
	{
		return false;
	}

	return IsAlive();
}

bool FBaseUnit::CanAttack(int32 cost) const
{
	int32 atkCost = mCurrentCombatWeapon.baseCost + cost;
	if (mCurrentTurnResources.ap < atkCost)
	{
		return false;
	}
	return true;
}

int32 FBaseUnit::GetActionCost()
{
	//TODO: not yet implemented
	return 1;
}

void FBaseUnit::DoAction()
{
	//TODO: not yet implemented
}

int32 FBaseUnit::GetHealth() const
{
	return mBaseCombatStats.health;
}

int FBaseUnit::GetMaxHealth() const
{
	return mBaseCombatStats.maxHealth;
}

int32 FBaseUnit::GetMovementPoints() const
{
	return mCurrentTurnResources.move;
}

int32 FBaseUnit::GetMaxMovementPoints() const
{
	return mBaseTurnResources.move;
}

bool FBaseUnit::SpendMovementPoints(int32 value)
{
	if (value <= 0)
	{
		return false;
	}

	if (mCurrentTurnResources.move < value)
	{
		return false;
	}

	mCurrentTurnResources.move -= value;
	return true;
}

int32 FBaseUnit::GetActionPoints() const
{
	return mCurrentTurnResources.ap;
}

int32 FBaseUnit::GetMaxActionPoints() const
{
	return mBaseTurnResources.ap;
}

bool FBaseUnit::SpendActionPoints(int32 value)
{
	if (value <= 0)
	{
		return false;
	}

	if (mCurrentTurnResources.ap < value)
	{
		return false;
	}

	mCurrentTurnResources.ap -= value;
	return true;
}

FBaseStatusEffect* FBaseUnit::AddStatusEffect(uint8 StatusId)
{
	if (HasStatusType(StatusId))
	{
		return nullptr;
	}

	for (uint8 i = 0; i < MAX_EFFECTS_PER_UNIT; ++i)
	{
		if (mStatusEffectsBuffer[i])
		{
			continue;
		}

		mStatusEffectsBuffer[i] = std::make_unique<FStatusEffect>(this, StatusId);
		FBaseStatusEffect* effect = mStatusEffectsBuffer[i].get();

		if (OnStatusAdded)
		{
			OnStatusAdded(this, effect->GetType());
		}

		return effect;
	}
	return nullptr;
}

bool FBaseUnit::RemoveStatusEffect(uint8 type)
{
	bool success = false;
	for (int i = 0; i < MAX_EFFECTS_PER_UNIT; ++i)
	{
		if (mStatusEffectsBuffer[i] && mStatusEffectsBuffer[i]->GetType() == type)
		{
			if (OnStatusRemoved)
			{
				OnStatusRemoved(this, type);
			}

			mStatusEffectsBuffer[i].reset();
			success = true;
			break;
		}
	}
	return success;
}

FBaseStatusEffect* FBaseUnit::GetStatusEffect(uint8 type) const
{
	for (int i = 0; i < MAX_EFFECTS_PER_UNIT; ++i)
	{
		if (mStatusEffectsBuffer[i] && mStatusEffectsBuffer[i]->GetType() == type)
		{
			return mStatusEffectsBuffer[i].get();
		}
	}

	return nullptr;
}

void FBaseUnit::GetStatusEffects(std::vector<FBaseStatusEffect*>& effects) const
{
	effects.clear();
	effects.reserve(MAX_EFFECTS_PER_UNIT);
	for (int i = 0; i < MAX_EFFECTS_PER_UNIT; ++i)
	{
		if (mStatusEffectsBuffer[i])
		{
			effects.push_back(mStatusEffectsBuffer[i].get());
		}
	}
}

void FBaseUnit::AddStatusTurnTicks()
{
	for (int i = 0; i < MAX_EFFECTS_PER_UNIT; ++i)
	{
		if (!mStatusEffectsBuffer[i])
		{
			continue;
		}
		if (OnStatusTriggered)
		{
			OnStatusTriggered(this, mStatusEffectsBuffer[i]->GetType());
		}
		mStatusEffectsBuffer[i]->AddTurnTick(this);
	}
}

bool FBaseUnit::HasStatusType(uint8 type) const
{
	for (int i = 0; i < MAX_EFFECTS_PER_UNIT; ++i)
	{
		if (mStatusEffectsBuffer[i] && mStatusEffectsBuffer[i]->GetType() == type)
		{
			return true;
		}
	}
	return false;
}

void FBaseUnit::ResetTurnStats()
{
	mCurrentTurnResources.move = mBaseTurnResources.move;
	mCurrentTurnResources.ap = mBaseTurnResources.ap;

	AddStatusTurnTicks();
}

void FBaseUnit::SetAttribute(const FName& name, int32 value)
{
	mAttributes[name] = value;
}

int32 FBaseUnit::GetAttribute(const FName& name, int32 defaultValue) const
{
	auto it = mAttributes.find(name);
	return it != mAttributes.end() ? it->second : defaultValue;
}

bool FBaseUnit::HasAttribute(const FName& name) const
{
	return mAttributes.find(name) != mAttributes.end();
}

const std::map<FName, int32>& FBaseUnit::GetAttributes() const
{
	return mAttributes;
}

const FName& FBaseUnit::GetTag() const
{
	return mTag;
}

void FBaseUnit::SetTag(const FName& tag)
{
	mTag = tag;
}

const FName& FBaseUnit::GetName() const
{
	return mName;
}

void FBaseUnit::SetName(const FName& name)
{
	mName = name;
}

int32 FBaseUnit::GetWeaponAttackCost() const
{
	return mCurrentCombatWeapon.baseCost; //TODO: get attack cost based on the weapon
}

int32 FBaseUnit::GetWeaponBaseDamage() const
{
	return mCurrentCombatWeapon.baseDamage;
}

int32 FBaseUnit::GetWeaponAttackRange() const
{
	return mCurrentCombatWeapon.range;
}

int32 FBaseUnit::GetVisionRange() const
{
	//TODO: implement something for the vision system, if will have one.
	return mCurrentTurnResources.move;
}

void FBaseUnit::ResetHealth()
{
	mBaseCombatStats.health = mBaseCombatStats.maxHealth;
}

void FBaseUnit::SetCombatStats(const FCombatStats& combatStats, const FTurnResources& turnResources)
{
	mBaseCombatStats = combatStats;
	mBaseTurnResources = turnResources;
	ResetTurnStats();
}

void FBaseUnit::SetCombatWeapon(const FCombatWeapon& combatWeapon)
{
	mCurrentCombatWeapon = combatWeapon;
}

int32 FBaseUnit::GetDistance(FGrid::FGridCoordinate position, FGrid::FGridCoordinate targetPosition)
{
	return std::max(1, std::abs(position.x - targetPosition.x) + std::abs(position.y - targetPosition.y));
}
