# 01 Bug fixes that block the game

Status: done (tests in `BugfixTests.cpp`, suite `Hellmarch.Battlefield.Bugfix.*`).

Source: `gameplay-rules.md` section 10. Fix only what the game needs. Each fix is its own small commit with a test.

## Tasks
- [x] **Move event carries the unit.** Add `unit` (pointer or id) and `from` to `FBattleMoveEvent` (`Public/Events/BattleMoveEvent.h`); fill them in `FBattleController::MoveUnit`/`MoveUnitRaw`. Needed by the renderer to animate a path.
  - Done when: `OnUnitMoved` listener can read who moved and from where; test `Bugfix.MoveEventCarriesUnitAndOriginTest`.
- [x] **Heal target check.** `HealUnitRaw` currently requires a hostile target. Require a friendly (non-hostile) target instead.
  - Done when: healing an ally works, healing an enemy is rejected.
- [x] **Current-team check on actions.** `DoUnitAction` must reject units that are not on the current team.
- [x] **Ally occupancy in movement cost.** `FBaseUnit::GetMovementCost` returns -1 when the destination holds an ally. Keep that, but add `GetMovementArea` option `passThroughAllies` so units can path through friends and still not end on them.
  - Done when: a unit can cross an ally to reach a free tile behind it; cannot stop on the ally.
- [x] **Winner default.** `GetWinner()` returns Enemy when nobody is left. Return `ETeams::Null` when both sides are dead.
- [x] **Stale tile occupancy.** `StartBattle` should clear unit occupancy on the grid so a grid can be reused for a new battle.
- [x] **Event naming.** Rename `BattleHealEvent` to `FBattleHealEvent` (keep an alias for one release).

## Files
`Public/Core/BattleController.h`, `Private/Core/BattleController.cpp`, `Private/Core/BaseUnit.cpp`, `Public/Events/*`, `docs/gameplay-rules.md` (remove fixed items from section 10).
