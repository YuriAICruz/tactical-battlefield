# 02 Unit attribute bag

The game stores its own stats (STR, SKL, SPD, LCK, DEF, RES, level, XP, class id...) on units. The DLL only stores them, never interprets them.

## Tasks
- [ ] Add `std::map<FName, int32> mAttributes` to `FBaseUnit` (`Public/Core/BaseUnit.h`).
- [ ] API: `SetAttribute(name, value)`, `GetAttribute(name, default = 0)`, `HasAttribute(name)`, `GetAttributes()` (const ref).
- [ ] Add a free-form `FName mTag` (class or archetype label) with getter and setter. Not interpreted.
- [ ] Add a unit display `name` field to `FBaseUnit` (today `FUnitData::name` is dropped after spawn). Copy it from `FUnitData` in `SpawnUnit`.
- [ ] Extend `FBattleController::FUnitData` with an optional `std::map<FName,int32> attributes` copied onto the unit at spawn.
- [ ] Tests: set/get/default, spawn copies attributes and name.

## Done when
A client can spawn a unit with arbitrary named integer attributes and read them back through the unit, with no change to existing combat behaviour.

## Files
`Public/Core/BaseUnit.h`, `Private/Core/BaseUnit.cpp`, `Public/Core/BattleController.h`, `Private/Core/BattleController.cpp`, `BattlefieldTests/Private/Tests/ActorTests.cpp`.
