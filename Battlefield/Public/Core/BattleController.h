#pragma once
#include "Platform/CoreTypes.h"
#include "BattleQueries.h"
#include "Core/Action.h"
#include "Core/BattlefieldData.h"
#include "Events/AttackResult.h"
#include "Events/BattleAttackEvent.h"
#include "Events/BattleDamageEvent.h"
#include "Events/BattleHealedEvent.h"
#include "Events/BattleMoveEvent.h"
#include "Events/FBattleHealEvent.h"

class BATTLEFIELD_API FBattleController
{
public:
	struct BATTLEFIELD_API FUnitData
	{
		FName name;
		ETeams team;
		FBaseUnit::FCombatStats combatStats;
		FBaseUnit::FTurnResources turnResources;
		FBaseUnit::FCombatWeapon weapon;
		FGrid::FGridCoordinate position;
		std::map<FName, int32> attributes; // optional, copied onto the unit at spawn
		std::vector<FWeaponDef> weapons; // optional; when not empty it replaces `weapon` (index 0 is equipped)
	};

	struct BATTLEFIELD_API FBattleStartData
	{
		std::vector<FUnitData> units;
	};

	//Battle lifecycle
	std::function<void(const FBattleStartData&)> OnBattleStarted;
	std::function<void(ETeams)> OnBattleEnded;
	std::function<void(ETeams)> OnWinnerDeclared;
	//Turn cycle
	std::function<void(ETeams)> OnTurnStarted;
	std::function<void(ETeams)> OnTurnEnded;
	//Unit lifecycle
	std::function<void(FBaseUnit*, const FGrid::FGridCoordinate& position)> OnUnitSpawned;
	std::function<void(const FBattleMoveEvent&)> OnUnitMoved;
	std::function<void(const FBattleAttackEvent&)> OnUnitDidAttacked;
	std::function<void(const FBattleHealEvent&)> OnUnitDidHealed;
	// Fired by DoUnitAction and PerformAction (kept one-argument for old callers).
	std::function<void(const FBaseUnit*)> OnUnitActionPerformed;
	// Fired by PerformAction right after OnUnitActionPerformed, with the id of the registered action.
	std::function<void(const FBaseUnit*, const FName& actionId)> OnActionPerformed;
	std::function<void(const FBaseUnit*)> OnUnitWaited;
	std::function<void(const FBattleDamageEvent&)> OnUnitDamaged;
	std::function<void(const FBattleHealedEvent&)> OnUnitHealed;
	std::function<void(const FBaseUnit*)> OnUnitDied;
	std::function<void(const FBaseUnit*, uint8)> OnStatusAdded;
	std::function<void(const FBaseUnit*, uint8)> OnStatusRemoved;
	std::function<void(const FBaseUnit*, uint8)> OnStatusTriggered;

	// Attack resolve hook: lets the client decide damage, hit and crit of AttackUnitWithWeapon. When unset the
	// built-in distance/aim formula is used (always a hit).
	// RULE: the hook (and PreviewAttack) may only call read-only queries. It must not move, spawn, attack,
	// heal, wait or end turns on this controller (asserted in debug builds). It must be pure for previews:
	// no RNG is consumed by the DLL, so any randomness belongs to the client.
	std::function<FAttackResult(FBaseUnit& attacker, FBaseUnit& target, const FWeaponDef& weapon)> OnResolveAttack;

	FBattleController(FGrid* grid);

	FBaseUnit* SpawnUnit(ETeams team,
	                     const FBaseUnit::FCombatStats& stats,
	                     const FBaseUnit::FTurnResources& resources,
	                     const FBaseUnit::FCombatWeapon& weapon,
	                     FGrid::FGridCoordinate position,
	                     const FName& name = FName(),
	                     const std::map<FName, int32>& attributes = {},
	                     const std::vector<FWeaponDef>& weapons = {});
	bool MoveUnit(FBaseUnit* Unit, FGrid::FGridCoordinate Destination) const;
	bool MoveUnitRaw(FBaseUnit* unit, FGrid::FGridCoordinate destination, int32 cost) const;
	bool AttackUnitWithWeapon(FBaseUnit* attacker, FBaseUnit* target) const;
	// Forecast of AttackUnitWithWeapon: the result the hook (or the default formula) gives, nothing is applied
	// and no event fires. Does not check team, AP or range. Unarmed or null units give {0, hit=false}.
	FAttackResult PreviewAttack(FBaseUnit* attacker, FBaseUnit* target) const;
	bool AttackUnitRaw(FBaseUnit* healer, FBaseUnit* target, int32 range, int32 cost, int32 damage) const;
	bool HealUnitRaw(FBaseUnit* attacker, FBaseUnit* target, int32 range, int32 cost, int32 value) const;
	int32 GetTurnNumber() const;

	static FBaseStatusEffect* ApplyStatus(FBaseUnit* attacker, FBaseUnit* target, int32 cost, uint8 statusId);
	static bool RemoveStatus(FBaseUnit* attacker, FBaseUnit* target, int32 cost, uint8 statusId);
	static void GetStatus(FBaseUnit* unit, std::vector<FBaseStatusEffect*>& effects);
	static bool HasStatus(FBaseUnit* unit, uint8 statusId);

	// Action registry. Registering an id that already exists replaces it. Empty ids are rejected.
	bool RegisterAction(const FActionDef& action);
	bool UnregisterAction(const FName& id);
	const std::vector<FActionDef>& GetActions() const;
	// Validates (unit non-null, its team's turn and able to act, action registered, canUse, enough AP), calls
	// execute(unit, target); on true spends apCost, fires OnUnitActionPerformed + OnActionPerformed, then, when
	// endsUnitTurn, waits the unit (WaitUnit, so OnUnitWaited fires). On any failure nothing is spent.
	// Unlike OnResolveAttack, execute may mutate the battle (it is the action).
	bool PerformAction(FBaseUnit* unit, const FName& actionId, FGrid::FGridCoordinate target = {}) const;
	// Deprecated: delegates to PerformAction(unit, "default") when a client registered that id; otherwise keeps
	// the old stub behaviour (cost 1, no effect, OnUnitActionPerformed).
	bool DoUnitAction(FBaseUnit* unit) const;
	// Ends the unit's phase. Fails for null/dead units, units not on the current team and units that already waited.
	bool WaitUnit(FBaseUnit* unit) const;
	// Opt-in (default false): when true, a unit that waited cannot move or act until its team's next turn.
	void SetRequireWait(bool require);
	bool GetRequireWait() const;
	bool StartBattle(const FBattleStartData& data);
	void EndTurn();
	const FBattleQueries* GetQueryController() const { return &mQueries; }
	const FGrid* GetGrid() const;

private:
	void UnitDied(const FBaseUnit* target) const;
	FAttackResult ResolveAttack(FBaseUnit* attacker, FBaseUnit* target, const FWeaponDef& weapon) const;

	std::vector<FActionDef> mActions;
	mutable bool mResolvingAttack = false; // true while OnResolveAttack runs (re-entrancy guard)

	int32 mTurnCount = 0;
	FBattleQueries mQueries;
	FBattlefieldData mBattlefield;
	FGrid* mpGrid;
	FTeamSystem mTeamSystem;
};
