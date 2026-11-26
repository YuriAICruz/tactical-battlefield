#pragma once
#include "Platform/CoreTypes.h"
#include "Core/GridEntity.h"
#include "Enums/Teams.h"
#include "StatusEffects/BaseStatusEffect.h"
#include "Grid.h"
#include "Core/Weapon.h"
#include <functional>
#include <memory>

#include "StatusEffects/StatusEffect.h"

static constexpr uint8 MAX_EFFECTS_PER_UNIT = 8;

class FTeamSystem;

class BATTLEFIELD_API FBaseUnit : public FGridEntity
{
public:
	struct FTurnResources
	{
		int32 ap = 6;
		int32 move = 5;

		FTurnResources operator+(const FTurnResources& other) const
		{
			FTurnResources result;

			result.ap = ap + other.ap;
			result.move = move + other.move;
			return result;
		}
	};

	struct FCombatStats
	{
		int32 maxHealth = 10;
		int32 health = 10;
		int32 armor = 0;
		int32 aim = 10;
		int32 dodge = 0;

		FCombatStats operator+(const FCombatStats& other) const
		{
			FCombatStats result;

			result.maxHealth = maxHealth + other.maxHealth;
			result.health = health + other.health;
			result.armor = armor + other.armor;
			result.aim = aim + other.aim;
			result.dodge = dodge + other.dodge;
			return result;
		}
	};

	struct FCombatWeapon
	{
		int32 baseDamage = 5;
		int32 baseCost = 1;
		int32 range = 5;
	};

public:
	std::function<void(const FBaseUnit*)> OnDeath;
	std::function<void(const FBaseUnit*, int32 damage)> OnReceiveDamage;
	std::function<void(const FBaseUnit*, int32 value)> OnReceivedHeal;
	std::function<void(const FBaseUnit*, uint8)> OnStatusAdded;
	std::function<void(const FBaseUnit*, uint8)> OnStatusRemoved;
	std::function<void(const FBaseUnit*, uint8)> OnStatusTriggered;

public:
	FBaseUnit(const uint32& unitId);
	virtual ~FBaseUnit() = default;

	bool IsEnemy(const FBaseUnit* actor) const;

	bool IsAlive() const;
	void ReceiveDamage(int32 damage);
	void ReceiveHeal(int32 heal);
	int32 CalculateWeaponAttackDamage(FGrid::FGridCoordinate position, FGrid::FGridCoordinate targetPosition) const;
	bool InRange(FGrid::FGridCoordinate position, FGrid::FGridCoordinate targetPosition) const;
	int32 GetWeaponAttackCost() const;
	int32 GetWeaponBaseDamage() const;
	int32 GetWeaponAttackRange() const; // max range of the equipped weapon (0 when unarmed)
	int32 GetWeaponMinRange() const; // min range of the equipped weapon (0 when unarmed)
	int32 GetVisionRange() const;

	// passThroughAllies: allies on the way can be crossed, but a destination holding any unit still returns -1.
	int32 GetMovementCost(const FGrid* grid, FGrid::FGridCoordinate destination, bool passThroughAllies = false) const;
	virtual bool CanMove(int32 cost = 1) const override;

	bool CanDoAction(int32 cost = 1) const;
	bool CanAttack(int32 cost = 0) const;
	int32 GetActionCost();
	void DoAction();

	int32 GetHealth() const;
	int32 GetMaxHealth() const;

	int32 GetMovementPoints() const;
	int32 GetMaxMovementPoints() const;
	bool SpendMovementPoints(int32 value);

	int32 GetActionPoints() const;
	int32 GetMaxActionPoints() const;
	bool SpendActionPoints(int32 value);

	FBaseStatusEffect* AddStatusEffect(uint8 StatusId);
	bool RemoveStatusEffect(uint8 type);
	FBaseStatusEffect* GetStatusEffect(uint8 type) const;
	void GetStatusEffects(std::vector<FBaseStatusEffect*>& effects) const;
	bool HasStatusType(uint8 type) const;

	void ResetTurnStats();

	// Turn state: a unit that waited is done for this phase (cleared by ResetTurnStats on its team's next turn).
	void Wait();
	bool HasWaited() const;
	// Alive, has not waited, and still has move or AP.
	bool CanAct() const;

	// Attribute bag: game-defined named integers (STR, SKL, level, XP...). Stored, never interpreted.
	void SetAttribute(const FName& name, int32 value);
	int32 GetAttribute(const FName& name, int32 defaultValue = 0) const;
	bool HasAttribute(const FName& name) const;
	const std::map<FName, int32>& GetAttributes() const;

	// Weapons: a unit owns a list and one is equipped. A legacy FCombatWeapon becomes the weapon at index 0.
	// Bad indexes return false. Removing the equipped weapon equips index 0 (if any is left).
	void AddWeapon(const FWeaponDef& weapon);
	bool RemoveWeapon(int32 index);
	const std::vector<FWeaponDef>& GetWeapons() const;
	bool EquipWeapon(int32 index);
	int32 GetEquippedWeaponIndex() const;
	// nullptr when the unit has no weapon.
	const FWeaponDef* GetEquippedWeapon() const;
	// Replaces the whole list and equips index 0.
	void SetWeapons(const std::vector<FWeaponDef>& weapons);

	// Free-form class/archetype label. Not interpreted.
	const FName& GetTag() const;
	void SetTag(const FName& tag);

	// Display name, copied from FUnitData at spawn.
	const FName& GetName() const;
	void SetName(const FName& name);

protected:
	void SetCombatStats(const FCombatStats& combatStats, const FTurnResources& turnResources);
	void SetCombatWeapon(const FCombatWeapon& combatWeapon);
	void ResetHealth();
	void AddStatusTurnTicks();
	static int32 GetDistance(FGrid::FGridCoordinate position, FGrid::FGridCoordinate targetPosition);

private:
	FCombatStats mBaseCombatStats;
	FTurnResources mBaseTurnResources;
	FTurnResources mCurrentTurnResources;
	std::vector<FWeaponDef> mWeapons;
	int32 mEquippedWeapon = 0;
	std::map<FName, int32> mAttributes;
	FName mTag;
	FName mName;
	bool mHasWaited = false;
	std::unique_ptr<FBaseStatusEffect> mStatusEffectsBuffer[MAX_EFFECTS_PER_UNIT];
};

class FUnit : public FBaseUnit
{
public:
	FUnit(const uint32& unitId, const FCombatStats& combatStats, const FTurnResources& turnResources,
	      const FCombatWeapon& combatWeapon, ETeams team):
		FBaseUnit(unitId)
	{
		SetCombatStats(combatStats, turnResources);
		SetCombatWeapon(combatWeapon);
		SetTeam(team);
		ResetTurnStats();
		ResetHealth();
	}
};
