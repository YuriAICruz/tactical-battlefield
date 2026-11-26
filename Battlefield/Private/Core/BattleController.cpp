#include "Core/BattleController.h"
#include <cassert>
#include "Core/TeamSystemHelper.h"
#include "Enums/Teams.h"
#include "Platform/BattlefieldLog.h"

FBattleController::FBattleController(FGrid* grid) :
	mQueries(&mTeamSystem, grid, &mBattlefield),
	mBattlefield(&mTeamSystem),
	mpGrid(grid)
{
}

FBaseUnit* FBattleController::SpawnUnit(ETeams team, const FBaseUnit::FCombatStats& stats,
                                        const FBaseUnit::FTurnResources& resources,
                                        const FBaseUnit::FCombatWeapon& weapon,
                                        FGrid::FGridCoordinate position,
                                        const FName& name,
                                        const std::map<FName, int32>& attributes)
{
	if (mpGrid->IsBlocked(position))
	{
		BF_LOG(Error, "FBattleController/SpawnUnit: Failed To Spawn position (%d, %d) blocked",
		       position.x, position.y);
		return nullptr;
	}

	FTroupe* troupe = mBattlefield.GetTroupe(team);
	assert(troupe);
	FBaseUnit* unit = troupe->CreateActor<FUnit>(stats, resources, weapon, team);
	if (unit == nullptr)
	{
		BF_LOG(Error, "FBattleController/SpawnUnit: troupe->CreateActor Failed");
		return nullptr;
	}

	unit->SetName(name);
	for (const auto& [attrName, attrValue] : attributes)
	{
		unit->SetAttribute(attrName, attrValue);
	}

	bool moved = mpGrid->MoveUnitToTile(position, unit);
	if (!moved)
	{
		troupe->DestroyActor(unit);
		BF_LOG(Error, "FBattleController/SpawnUnit: failed to move actor");
		return nullptr;
	}
	if (OnUnitSpawned)
	{
		OnUnitSpawned(unit, position);
	}
	unit->OnDeath = [this](const FBaseUnit* target)
	{
		UnitDied(target);
	};
	unit->OnReceiveDamage = [this](const FBaseUnit* target, int32 damage)
	{
		if (OnUnitDamaged)
		{
			OnUnitDamaged({target, damage});
		}
	};
	unit->OnReceivedHeal = [this](const FBaseUnit* target, int32 value)
	{
		if (OnUnitHealed)
		{
			OnUnitHealed({target, value});
		}
	};
	unit->OnStatusAdded = [this](const FBaseUnit* target, uint8 effect)
	{
		if (OnStatusAdded)
		{
			OnStatusAdded(target, effect);
		}
	};
	unit->OnStatusRemoved = [this](const FBaseUnit* target, uint8 effect)
	{
		if (OnStatusRemoved)
		{
			OnStatusRemoved(target, effect);
		}
	};
	unit->OnStatusTriggered = [this](const FBaseUnit* target, uint8 effect)
	{
		if (OnStatusTriggered)
		{
			OnStatusTriggered(target, effect);
		}
	};

	return unit;
}

bool FBattleController::MoveUnit(FBaseUnit* unit, FGrid::FGridCoordinate destination) const
{
	if (!mTeamSystem.CanAct(unit))
	{
		return false;
	}

	int32 cost = unit->GetMovementCost(mpGrid, destination, true);
	if (cost < 0 || !unit->CanMove(cost))
	{
		return false;
	}
	const FGrid::FGridCoordinate from = mpGrid->GetUnitPosition(unit);
	if (mpGrid->MoveUnitToTile(destination, unit))
	{
		unit->SpendMovementPoints(cost);
		if (OnUnitMoved)
		{
			OnUnitMoved({unit, from, destination});
		}
		return true;
	}

	return false;
}

bool FBattleController::MoveUnitRaw(FBaseUnit* unit, FGrid::FGridCoordinate destination, int32 cost) const
{
	if (!mTeamSystem.CanAct(unit))
	{
		return false;
	}

	if (cost < 0 || !unit->CanMove(cost))
	{
		return false;
	}
	const FGrid::FGridCoordinate from = mpGrid->GetUnitPosition(unit);
	if (mpGrid->MoveUnitToTile(destination, unit))
	{
		unit->SpendMovementPoints(cost);
		if (OnUnitMoved)
		{
			OnUnitMoved({unit, from, destination});
		}
		return true;
	}

	return false;
}

bool FBattleController::AttackUnitWithWeapon(FBaseUnit* attacker, FBaseUnit* target) const
{
	if (!attacker || !target)
	{
		return false;
	}

	if (!TeamSystemHelper::IsHostile(attacker, target))
	{
		return false;
	}

	if (!mTeamSystem.CanAct(attacker))
	{
		return false;
	}

	int32 cost = attacker->GetWeaponAttackCost();
	if (!attacker->CanDoAction(cost))
	{
		return false;
	}
	FGrid::FGridCoordinate attackerPos = mpGrid->GetUnitPosition(attacker);
	FGrid::FGridCoordinate targetPos = mpGrid->GetUnitPosition(target);
	if (!attacker->InRange(attackerPos, targetPos))
	{
		return false;
	}
	int32 damage = attacker->CalculateWeaponAttackDamage(attackerPos, targetPos);
	target->ReceiveDamage(damage);
	attacker->SpendActionPoints(cost);

	if (OnUnitDidAttacked)
	{
		OnUnitDidAttacked({attacker, target, damage});
	}
	return true;
}

bool FBattleController::AttackUnitRaw(FBaseUnit* healer, FBaseUnit* target, int32 range, int32 cost, int32 damage) const
{
	if (!healer || !target)
	{
		return false;
	}

	if (!TeamSystemHelper::IsHostile(healer, target))
	{
		return false;
	}

	if (!mTeamSystem.CanAct(healer))
	{
		return false;
	}

	if (!healer->CanDoAction(cost))
	{
		return false;
	}
	FGrid::FGridCoordinate attackerPos = mpGrid->GetUnitPosition(healer);
	FGrid::FGridCoordinate targetPos = mpGrid->GetUnitPosition(target);
	if (mpGrid->GetDistance(attackerPos, targetPos) > range)
	{
		return false;
	}
	target->ReceiveDamage(damage);
	healer->SpendActionPoints(cost);

	if (OnUnitDidAttacked)
	{
		OnUnitDidAttacked({healer, target, damage});
	}
	return true;
}

bool FBattleController::HealUnitRaw(FBaseUnit* attacker, FBaseUnit* target, int32 range, int32 cost, int32 value) const
{
	if (!attacker || !target)
	{
		return false;
	}

	if (TeamSystemHelper::IsHostile(attacker, target))
	{
		return false;
	}

	if (!mTeamSystem.CanAct(attacker))
	{
		return false;
	}

	if (!attacker->CanDoAction(cost))
	{
		return false;
	}
	FGrid::FGridCoordinate attackerPos = mpGrid->GetUnitPosition(attacker);
	FGrid::FGridCoordinate targetPos = mpGrid->GetUnitPosition(target);
	if (mpGrid->GetDistance(attackerPos, targetPos) > range)
	{
		return false;
	}
	target->ReceiveHeal(value);
	attacker->SpendActionPoints(cost);

	if (OnUnitDidHealed)
	{
		OnUnitDidHealed({attacker, target, value});
	}
	return true;
}

int32 FBattleController::GetTurnNumber() const
{
	return mTurnCount;
}

FBaseStatusEffect* FBattleController::ApplyStatus(FBaseUnit* attacker, FBaseUnit* target, int32 cost, uint8 statusId)
{
	if (cost < 0 || !attacker->CanDoAction(cost))
	{
		return nullptr;
	}

	if (FBaseStatusEffect* statusEffects = target->AddStatusEffect(statusId))
	{
		attacker->SpendMovementPoints(cost);
		return statusEffects;
	}

	return nullptr;
}

bool FBattleController::RemoveStatus(FBaseUnit* attacker, FBaseUnit* target, int32 cost, uint8 statusId)
{
	if (cost < 0 || !attacker->CanDoAction(cost))
	{
		return false;
	}

	if (target->RemoveStatusEffect(statusId))
	{
		attacker->SpendMovementPoints(cost);
		return true;
	}

	return false;
}

void FBattleController::GetStatus(FBaseUnit* unit, std::vector<FBaseStatusEffect*>& effects)
{
	unit->GetStatusEffects(effects);
}

bool FBattleController::HasStatus(FBaseUnit* unit, uint8 statusId)
{
	return unit->HasStatusType(statusId);
}

bool FBattleController::DoUnitAction(FBaseUnit* unit) const
{
	if (!mTeamSystem.CanAct(unit))
	{
		return false;
	}

	int32 cost = unit->GetActionCost();
	if (!unit->CanDoAction(cost))
	{
		return false;
	}
	unit->DoAction();
	unit->SpendActionPoints(cost);
	if (OnUnitActionPerformed)
	{
		OnUnitActionPerformed(unit);
	}
	return true;
}

bool FBattleController::WaitUnit(FBaseUnit* unit) const
{
	if (!unit || !unit->IsAlive() || unit->HasWaited())
	{
		return false;
	}

	if (!mTeamSystem.CanAct(unit, false))
	{
		return false;
	}

	unit->Wait();
	if (OnUnitWaited)
	{
		OnUnitWaited(unit);
	}
	return true;
}

void FBattleController::SetRequireWait(bool require)
{
	mTeamSystem.SetRequireWait(require);
}

bool FBattleController::GetRequireWait() const
{
	return mTeamSystem.GetRequireWait();
}

bool FBattleController::StartBattle(const FBattleStartData& data)
{
	mBattlefield.Clear();
	mpGrid->ClearUnits();
	for (auto unit : data.units)
	{
		if (unit.team == ETeams::Null || unit.team == ETeams::Length)
		{
			BF_LOG(Error, "FBattleController/StartBattle: Invalid Team %d", static_cast<int>(unit.team));
			return false;
		}
		FBaseUnit* createdUnit = SpawnUnit(unit.team, unit.combatStats, unit.turnResources, unit.weapon, unit.position,
		                                      unit.name, unit.attributes);
		if (!createdUnit)
		{
			BF_LOG(Error, "FBattleController/StartBattle: Failed To create Unit");
			return false;
		}
	}

	if (OnBattleStarted)
	{
		OnBattleStarted(data);
	}
	return true;
}

void FBattleController::EndTurn()
{
	if (OnTurnEnded)
	{
		OnTurnEnded(mTeamSystem.GetCurrentTeam());
	}

	mTeamSystem.EndTurn();
	mTurnCount++;
	if (OnTurnStarted)
	{
		OnTurnStarted(mTeamSystem.GetCurrentTeam());
	}
}

const FGrid* FBattleController::GetGrid() const
{
	return mpGrid;
}

void FBattleController::UnitDied(const FBaseUnit* target) const
{
	mpGrid->RemoveUnitFromTile(target);
	if (OnUnitDied)
	{
		OnUnitDied(target);
	}
	if (mQueries.HasWinner())
	{
		if (OnWinnerDeclared)
		{
			OnWinnerDeclared(mQueries.GetWinner());
		}
		if (OnBattleEnded)
		{
			OnBattleEnded(mQueries.GetWinner());
		}
	}
}
