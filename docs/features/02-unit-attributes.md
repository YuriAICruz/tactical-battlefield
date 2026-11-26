# 02 Unit attribute bag

The game stores its own stats (STR, SKL, SPD, LCK, DEF, RES, level, XP, class id...) on units. The DLL only stores them, never interprets them.

## Tasks
- [x] Add `std::map<FName, int32> mAttributes` to `FBaseUnit` (`Public/Core/BaseUnit.h`).
- [x] API: `SetAttribute(name, value)`, `GetAttribute(name, default = 0)`, `HasAttribute(name)`, `GetAttributes()` (const ref).
- [x] Add a free-form `FName mTag` (class or archetype label) with getter and setter. Not interpreted.
- [x] Add a unit display `name` field to `FBaseUnit` (today `FUnitData::name` is dropped after spawn). Copy it from `FUnitData` in `SpawnUnit`.
- [x] Extend `FBattleController::FUnitData` with an optional `std::map<FName,int32> attributes` copied onto the unit at spawn.
- [x] Tests: set/get/default, spawn copies attributes and name.

## Done when
A client can spawn a unit with arbitrary named integer attributes and read them back through the unit, with no change to existing combat behaviour.

## Notes
- `FBattleController::SpawnUnit` gained two trailing defaulted parameters, `name` and `attributes`, so existing callers compile unchanged. `StartBattle` passes `FUnitData::name` / `attributes` through.
- `FUnitData::attributes` is the last member, so existing brace-initialised `FUnitData` literals stay valid.
- `GetName()/SetName()` and `GetTag()/SetTag()` live on `FBaseUnit`; `mTag` is only settable via `SetTag` (not part of `FUnitData`).

## Files
`Public/Core/BaseUnit.h`, `Private/Core/BaseUnit.cpp`, `Public/Core/BattleController.h`, `Private/Core/BattleController.cpp`, `BattlefieldTests/Private/Tests/ActorTests.cpp`.
