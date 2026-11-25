#pragma once
#include "Platform/CoreTypes.h"
#include "BattleQueries.h"
#include "Core/BattlefieldData.h"
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
	std::function<void(const BattleHealEvent&)> OnUnitDidHealed;
	std::function<void(const FBaseUnit*)> OnUnitActionPerformed;
	std::function<void(const FBattleDamageEvent&)> OnUnitDamaged;
	std::function<void(const FBattleHealedEvent&)> OnUnitHealed;
	std::function<void(const FBaseUnit*)> OnUnitDied;
	std::function<void(const FBaseUnit*, uint8)> OnStatusAdded;
	std::function<void(const FBaseUnit*, uint8)> OnStatusRemoved;
	std::function<void(const FBaseUnit*, uint8)> OnStatusTriggered;

	FBattleController(FGrid* grid);

	FBaseUnit* SpawnUnit(ETeams team,
	                     const FBaseUnit::FCombatStats& stats,
	                     const FBaseUnit::FTurnResources& resources,
	                     const FBaseUnit::FCombatWeapon& weapon,
	                     FGrid::FGridCoordinate position);
	bool MoveUnit(FBaseUnit* Unit, FGrid::FGridCoordinate Destination) const;
	bool MoveUnitRaw(FBaseUnit* unit, FGrid::FGridCoordinate destination, int32 cost) const;
	bool AttackUnitWithWeapon(FBaseUnit* attacker, FBaseUnit* target) const;
	bool AttackUnitRaw(FBaseUnit* healer, FBaseUnit* target, int32 range, int32 cost, int32 damage) const;
	bool HealUnitRaw(FBaseUnit* attacker, FBaseUnit* target, int32 range, int32 cost, int32 value) const;
	int32 GetTurnNumber() const;

	static FBaseStatusEffect* ApplyStatus(FBaseUnit* attacker, FBaseUnit* target, int32 cost, uint8 statusId);
	static bool RemoveStatus(FBaseUnit* attacker, FBaseUnit* target, int32 cost, uint8 statusId);
	static void GetStatus(FBaseUnit* unit, std::vector<FBaseStatusEffect*>& effects);
	static bool HasStatus(FBaseUnit* unit, uint8 statusId);

	bool DoUnitAction(FBaseUnit* unit) const;
	bool StartBattle(const FBattleStartData& data);
	void EndTurn();
	const FBattleQueries* GetQueryController() const { return &mQueries; }
	const FGrid* GetGrid() const;

private:
	void UnitDied(const FBaseUnit* target) const;

	int32 mTurnCount = 0;
	FBattleQueries mQueries;
	FBattlefieldData mBattlefield;
	FGrid* mpGrid;
	FTeamSystem mTeamSystem;
};
