# 05 Weapon definitions and range bands

Replace the single hard-wired `FCombatWeapon` with data-driven weapons. The DLL never knows weapon kinds; it stores range and an opaque property map.

## Tasks
- [x] New `FWeaponDef` in `Public/Core/Weapon.h`: `FName id`, `FName name`, `int32 minRange`, `int32 maxRange`, `int32 baseDamage`, `int32 apCost`, `int32 typeTag` (opaque), `std::map<FName,int32> properties` (opaque, for might, hit, weight, durability...).
- [x] Add a per-unit weapon list to `FBaseUnit`: `AddWeapon`, `RemoveWeapon`, `GetWeapons`, `EquipWeapon(index)`, `GetEquippedWeapon()`.
- [x] Back-compat: existing `FCombatWeapon` converts to an `FWeaponDef` at index 0 so current code and tests behave the same.
- [x] Range bands: update `FBaseUnit::CanAttack` and `GetAttackableTargets` to require `minRange <= distance <= maxRange`.
- [x] `FBattleQueries::GetAttackArea(unit, fromTile)`: tiles a unit could hit from a given tile with the equipped weapon (used for the red overlay and AI).
- [x] `FBattleQueries::GetAttackableTargetsFrom(unit, tile)`: targets reachable if the unit stood on `tile`.
- [x] `FUnitData` accepts an optional weapon list for spawning.
- [x] Tests: range 1-1, range 2-2 (cannot hit adjacent), range 1-2, switching equipped weapon changes targets, old single-weapon path unchanged.

## Done when
A client can give a unit several weapons with different range bands and the DLL reports the correct attackable targets and attack area for the equipped one.

## Files
New `Public/Core/Weapon.h`; `BaseUnit.h/.cpp`, `BattleQueries.h/.cpp`, `BattleController.h/.cpp`, `ActorTests.cpp`, `BattlefieldQueryTests.cpp`.

## Notes
- `FWeaponDef` (`Public/Core/Weapon.h`) is header-only: `id`, `name`, `minRange`, `maxRange` (inclusive Manhattan band), `baseDamage`, `apCost`, opaque `typeTag` and `properties`. `IsInRange(distance)` checks the band.
- `FBaseUnit` owns `std::vector<FWeaponDef>` plus an equipped index. `AddWeapon`, `RemoveWeapon(i)` (the equipped index follows the weapon; removing the equipped one equips index 0), `EquipWeapon(i)`, `SetWeapons(list)`, `GetWeapons()`, `GetEquippedWeapon()` (`nullptr` when unarmed).
- Back-compat: a legacy `FCombatWeapon` becomes the single weapon at index 0 with band `1..range`, so existing code and tests behave the same. `GetWeaponAttackCost/BaseDamage/AttackRange` read the equipped weapon; new `GetWeaponMinRange()`.
- An unarmed unit (empty weapon list) cannot attack: `CanAttack()` and `InRange()` are false, and `AttackUnitWithWeapon` fails without spending AP.
- `CanAttack(cost)` still only checks AP (plus "has a weapon"); distance is checked by `InRange` / the queries, because it takes no position.
- `FBattleQueries::GetAttackArea(unit, tile, out)` and `GetAttackableTargetsFrom(unit, tile, out)`; `GetAttackableTargets` now delegates to the latter from the unit's own tile. The area is a threat overlay: it ignores units and blocked tiles and is clipped to the grid.
- `FUnitData::weapons` and the new trailing `SpawnUnit(..., weapons)` parameter: when non-empty they replace the legacy weapon and index 0 is equipped.
- Tests: `ActorTests.cpp` (`LegacyWeaponBecomesIndexZeroTest`, `AddEquipRemoveWeaponTest`, `UnarmedUnitCannotAttackTest`, `WeaponRangeBandInRangeTest`, `WeaponOpaqueDataIsStoredTest`), `BattlefieldQueryTests.cpp` (`AttackAreaRangeOneTest`, `AttackAreaExactRangeTwoTest`, `AttackAreaBandOneToTwoTest`, `AttackAreaClippedByGridEdgeTest`, `AttackAreaUnarmedIsEmptyTest`, `AttackableTargetsRespectMinRangeTest`, `SwitchingWeaponChangesTargetsTest`, `AttackableTargetsFromHypotheticalTileTest`, `LegacyWeaponTargetsUnchangedTest`), `BattlefieldControllerTests.cpp` (`AttackRespectsMinRangeTest`, `AttackUsesEquippedWeaponDamageTest`, `UnarmedControllerAttackFailsTest`, `StartBattleCopiesUnitDataWeaponsTest`).
