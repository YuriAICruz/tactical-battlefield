# 07 Custom action registry

`DoUnitAction` is a stub (cost always 1, `DoAction` empty). Replace it with a registry the client fills, so Attack, Wait, Heal, Rescue, Items, etc. are defined in Lua, not C++.

## Tasks
- [x] New `FActionDef` in `Public/Core/Action.h`: `FName id`, `FName name`, `int32 apCost`, `bool endsUnitTurn`, and callbacks:
  - `std::function<bool(const FBaseUnit&)> canUse`
  - `std::function<void(const FBaseUnit&, std::vector<FGridCoordinate>& out)> getTargets`
  - `std::function<bool(FBaseUnit&, FGridCoordinate target)> execute`
- [x] `FBattleController::RegisterAction(FActionDef)`, `UnregisterAction(id)`, `GetActions()`.
- [x] `FBattleQueries::GetAvailableActions(unit)`: registered actions where `canUse` is true and the unit has the AP.
- [x] `FBattleController::PerformAction(unit, actionId, target)`: validates team and AP, calls `execute`, spends AP, applies `endsUnitTurn` through feature 04, fires `OnUnitActionPerformed` with the action id.
- [x] Remove or deprecate the stub `DoUnitAction`; keep it delegating to action id "default" for old callers.
- [x] Make `AddStatusEffect` accept a client-created effect (`std::unique_ptr<FBaseStatusEffect>`) so Lua-defined effects are possible. Keep the old `uint8` overload.
- [x] Tests: register and list, `canUse` filtering, AP spent, `execute` failure spends nothing, ends-turn flag, wrong team rejected.

## Done when
A client registers an action with callbacks and the controller lists, validates, executes and reports it with no built-in knowledge of what the action does.

## Files
New `Public/Core/Action.h`; `BattleController.h/.cpp`, `BattleQueries.h/.cpp`, `BaseUnit.h/.cpp`, `BattlefieldControllerTests.cpp`.

## Notes
- `FActionDef` lives in `Public/Core/Action.h` (header-only). All three callbacks are optional: no `canUse` = always usable, no `execute` = succeeds with no effect (enough for a "Wait" that only ends the unit's turn), `getTargets` is for UIs and AI and the DLL never calls it.
- `PerformAction` does not validate the target against `getTargets`; `execute` decides whether the target is acceptable and returns `false` to cancel. It runs on a copy of the definition, so an action may register or unregister actions.
- The one-argument `OnUnitActionPerformed(const FBaseUnit*)` is kept for old callers; the id is delivered by the new `OnActionPerformed(unit, id)` (fired right after it), instead of changing the old signature.
- `DoUnitAction` delegates to the action id `"default"` (`kDefaultActionId`) when registered; otherwise the old stub is unchanged (cost 1, no effect).
- `FBattleQueries` now receives a pointer to the controller's registry, so `GetAvailableActions` needs no back-reference to the controller. `GetValidActions` (flags) is untouched.
- `FBaseUnit::AddStatusEffect(std::unique_ptr<FBaseStatusEffect>)` takes ownership; it returns `nullptr` (destroying the effect) for a null effect, an effect built for another unit, a duplicate type or full slots. The `uint8` overload now delegates to it.
- Tests: `BattlefieldControllerTests.cpp` (`RegisterAndListActionsTest`, `PerformActionSpendsApAndFiresEventsTest`, `PerformActionExecuteFailureSpendsNothingTest`, `PerformActionEndsUnitTurnTest`, `PerformActionRejectsInvalidCallsTest`, `PerformActionMayMutateBattleTest`, `DoUnitActionDelegatesToDefaultActionTest`, `GetTargetsCallbackIsReachableTest`), `BattlefieldQueryTests.cpp` (`GetAvailableActionsFiltersByCanUseAndApTest`, `GetAvailableActionsDeadOrNullTest`), `ActorTests.cpp` (`AddClientStatusEffectTest`, `AddClientStatusEffectRejectsInvalidTest`, `AddClientStatusEffectRespectsSlotLimitTest`).
