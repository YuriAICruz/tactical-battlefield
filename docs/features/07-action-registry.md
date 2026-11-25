# 07 Custom action registry

`DoUnitAction` is a stub (cost always 1, `DoAction` empty). Replace it with a registry the client fills, so Attack, Wait, Heal, Rescue, Items, etc. are defined in Lua, not C++.

## Tasks
- [ ] New `FActionDef` in `Public/Core/Action.h`: `FName id`, `FName name`, `int32 apCost`, `bool endsUnitTurn`, and callbacks:
  - `std::function<bool(const FBaseUnit&)> canUse`
  - `std::function<void(const FBaseUnit&, std::vector<FGridCoordinate>& out)> getTargets`
  - `std::function<bool(FBaseUnit&, FGridCoordinate target)> execute`
- [ ] `FBattleController::RegisterAction(FActionDef)`, `UnregisterAction(id)`, `GetActions()`.
- [ ] `FBattleQueries::GetAvailableActions(unit)`: registered actions where `canUse` is true and the unit has the AP.
- [ ] `FBattleController::PerformAction(unit, actionId, target)`: validates team and AP, calls `execute`, spends AP, applies `endsUnitTurn` through feature 04, fires `OnUnitActionPerformed` with the action id.
- [ ] Remove or deprecate the stub `DoUnitAction`; keep it delegating to action id "default" for old callers.
- [ ] Make `AddStatusEffect` accept a client-created effect (`std::unique_ptr<FBaseStatusEffect>`) so Lua-defined effects are possible. Keep the old `uint8` overload.
- [ ] Tests: register and list, `canUse` filtering, AP spent, `execute` failure spends nothing, ends-turn flag, wrong team rejected.

## Done when
A client registers an action with callbacks and the controller lists, validates, executes and reports it with no built-in knowledge of what the action does.

## Files
New `Public/Core/Action.h`; `BattleController.h/.cpp`, `BattleQueries.h/.cpp`, `BaseUnit.h/.cpp`, `BattlefieldControllerTests.cpp`.
