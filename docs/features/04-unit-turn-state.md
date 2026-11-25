# 04 Unit turn state (acted / waited)

Fire Emblem units act once per phase and then "wait". Today a unit can act as long as it has AP or move.

## Tasks
- [ ] Add `bool mHasWaited` to `FBaseUnit` with `Wait()`, `HasWaited()`.
- [ ] `FBaseUnit::CanAct()` returns false once waited, regardless of AP and move.
- [ ] `FTeamSystem::EndTurn` clears `mHasWaited` for living units of the incoming team (same place AP and move refresh).
- [ ] `FBattleController::WaitUnit(unit)`: validates current team, sets waited, fires a new `OnUnitWaited` event.
- [ ] `FBattleQueries::GetActableUnits(team)` returns living, not-waited units of a team.
- [ ] Expose `FBattleQueries::AllUnitsWaited(team)` so the game can auto-end the phase.
- [ ] Keep it opt-in: a controller flag `SetRequireWait(bool)` (default false) so the current sample game is unchanged.
- [ ] Tests: wait blocks move and attack, flag cleared on next team turn, flag off keeps old behaviour.

## Done when
With `SetRequireWait(true)`, a unit that waited cannot move or attack until its team's next turn.

## Files
`Public/Core/BaseUnit.h`, `Public/Core/TeamSystem.h`, `Public/Core/BattleController.h`, `Public/Core/BattleQueries.h` and matching `.cpp`, `BattlefieldFlowTests.cpp`.
