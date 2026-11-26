# 04 Unit turn state (acted / waited)

Fire Emblem units act once per phase and then "wait". Today a unit can act as long as it has AP or move.

## Tasks
- [x] Add `bool mHasWaited` to `FBaseUnit` with `Wait()`, `HasWaited()`.
- [x] `FBaseUnit::CanAct()` returns false once waited, regardless of AP and move.
- [x] `FTeamSystem::EndTurn` clears `mHasWaited` for living units of the incoming team (same place AP and move refresh).
- [x] `FBattleController::WaitUnit(unit)`: validates current team, sets waited, fires a new `OnUnitWaited` event.
- [x] `FBattleQueries::GetActableUnits(team)` returns living, not-waited units of a team.
- [x] Expose `FBattleQueries::AllUnitsWaited(team)` so the game can auto-end the phase.
- [x] Keep it opt-in: a controller flag `SetRequireWait(bool)` (default false) so the current sample game is unchanged.
- [x] Tests: wait blocks move and attack, flag cleared on next team turn, flag off keeps old behaviour.

## Done when
With `SetRequireWait(true)`, a unit that waited cannot move or attack until its team's next turn.

## Files
`Public/Core/BaseUnit.h`, `Public/Core/TeamSystem.h`, `Public/Core/BattleController.h`, `Public/Core/BattleQueries.h` and matching `.cpp`, `BattlefieldFlowTests.cpp`.

## Notes
- `FBaseUnit::Wait()` / `HasWaited()` / `CanAct()` (alive, not waited, and move or AP left). `ResetTurnStats()` clears the flag, so `FTeamSystem::EndTurn` clears it together with the AP/move refresh for the incoming team's living units.
- `FBattleController::SetRequireWait(bool)` (default `false`) is forwarded to `FTeamSystem`; when on, `FTeamSystem::CanAct` rejects waited units, which blocks `MoveUnit`, `MoveUnitRaw`, `AttackUnitWithWeapon`, `AttackUnitRaw`, `HealUnitRaw` and `DoUnitAction`.
- `WaitUnit(unit)` fails for null, dead, already-waited units and units outside the current team; on success it sets the flag and raises `OnUnitWaited(const FBaseUnit*)`. It works with the flag off too (the flag is only recorded, nothing is blocked), so a game can adopt it before enabling enforcement.
- Queries: `GetActableUnits(team, out)` returns living units that have not waited; `AllUnitsWaited(team)` is true when that list is empty (including a team with no living units).
- Tests (`BattlefieldFlowTests.cpp`): `UnitCanActFalseAfterWaitRegardlessOfResourcesTest`, `WaitUnitSetsWaitedAndFiresEventTest`, `WaitUnitRejectsInvalidCasesTest`, `WaitBlocksMoveAndAttackWhenRequiredTest`, `WaitedUnitDoesNotBlockOtherUnitsTest`, `WaitClearedOnTeamNextTurnTest`, `WaitHasNoEffectOnActionsWhenFlagOffTest`, `GetActableUnitsExcludesWaitedAndDeadTest`, `AllUnitsWaitedTest`, `AllUnitsWaitedIgnoresDeadUnitsTest`.
