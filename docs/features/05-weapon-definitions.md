# 05 Weapon definitions and range bands

Replace the single hard-wired `FCombatWeapon` with data-driven weapons. The DLL never knows weapon kinds; it stores range and an opaque property map.

## Tasks
- [ ] New `FWeaponDef` in `Public/Core/Weapon.h`: `FName id`, `FName name`, `int32 minRange`, `int32 maxRange`, `int32 baseDamage`, `int32 apCost`, `int32 typeTag` (opaque), `std::map<FName,int32> properties` (opaque, for might, hit, weight, durability...).
- [ ] Add a per-unit weapon list to `FBaseUnit`: `AddWeapon`, `RemoveWeapon`, `GetWeapons`, `EquipWeapon(index)`, `GetEquippedWeapon()`.
- [ ] Back-compat: existing `FCombatWeapon` converts to an `FWeaponDef` at index 0 so current code and tests behave the same.
- [ ] Range bands: update `FBaseUnit::CanAttack` and `GetAttackableTargets` to require `minRange <= distance <= maxRange`.
- [ ] `FBattleQueries::GetAttackArea(unit, fromTile)`: tiles a unit could hit from a given tile with the equipped weapon (used for the red overlay and AI).
- [ ] `FBattleQueries::GetAttackableTargetsFrom(unit, tile)`: targets reachable if the unit stood on `tile`.
- [ ] `FUnitData` accepts an optional weapon list for spawning.
- [ ] Tests: range 1-1, range 2-2 (cannot hit adjacent), range 1-2, switching equipped weapon changes targets, old single-weapon path unchanged.

## Done when
A client can give a unit several weapons with different range bands and the DLL reports the correct attackable targets and attack area for the equipped one.

## Files
New `Public/Core/Weapon.h`; `BaseUnit.h/.cpp`, `BattleQueries.h/.cpp`, `BattleController.h/.cpp`, `ActorTests.cpp`, `BattlefieldQueryTests.cpp`.
