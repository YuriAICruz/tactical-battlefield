# 06 Attack resolve hook

Hit chance, crits, weapon triangle, counters and double attacks are game design. The DLL lets the client decide the outcome of an attack, then applies it through the normal event path.

## Tasks
- [x] New `FAttackResult` in `Public/Events/`: `int32 damage`, `bool hit`, `bool critical`, `int32 userValue` (opaque).
- [x] New callback on `FBattleController`: `std::function<FAttackResult(FBaseUnit& attacker, FBaseUnit& target, const FWeaponDef& weapon)> OnResolveAttack`. When unset, the existing distance and aim formula is used.
- [x] `AttackUnitWithWeapon` calls the hook, then applies damage via the existing `AttackUnitRaw` internals so `OnUnitDidAttacked`, `OnUnitDamaged` and `OnUnitDied` fire as before.
- [x] Include `hit` and `critical` and `userValue` in `FBattleAttackEvent` so the UI can show "Miss" or "Critical".
- [x] A miss spends AP, fires the attack event with damage 0, and does not fire `OnUnitDamaged`.
- [x] Re-entrancy rule: the hook may call read-only queries but not mutate the battle. Document it, and assert in debug builds.
- [x] `FBattleController::PreviewAttack(attacker, target)` returns the `FAttackResult` from the hook without applying it. Used for the battle forecast box. The hook must be pure for previews (no RNG consumption in the DLL; RNG belongs to the client).
- [x] Tests: default formula unchanged, hook damage applied, miss path, event payload, kill triggers death event.

## Done when
A client can set one callback and fully own how damage, hit and crit are computed, with events and death handling still working.

## Files
`Public/Core/BattleController.h/.cpp`, `Public/Events/BattleAttackEvent.h`, new `Public/Events/AttackResult.h`, `BattlefieldControllerTests.cpp`.

## Notes
- `FAttackResult` (`Public/Events/AttackResult.h`): `damage`, `hit` (default true), `critical`, `userValue`. `FBattleAttackEvent` gained `Hit`, `Critical`, `UserValue` as trailing defaulted constructor arguments, so old `{attacker, target, damage}` constructions still compile and mean "hit".
- `AttackUnitWithWeapon` validates first (team, AP, range, weapon), then calls the hook (or the default formula, always a hit), applies damage with `ReceiveDamage` (so `OnUnitDamaged`, death and winner events fire as before), spends AP, then fires `OnUnitDidAttacked`. A miss or 0 damage never reaches `ReceiveDamage`.
- Re-entrancy: a `mResolvingAttack` flag is set while the hook runs; `SpawnUnit`, `MoveUnit`, `MoveUnitRaw`, `AttackUnitWithWeapon`, `AttackUnitRaw`, `HealUnitRaw`, `WaitUnit` and `EndTurn` assert it is clear. The assert only exists in debug builds, so it is not covered by an automated test.
- `PreviewAttack(attacker, target)` does not check team, AP or range (a forecast may ask about hypothetical fights); null or unarmed gives `{0, hit=false}`.
- Tests (`BattlefieldControllerTests.cpp`): `ResolveHookUnsetUsesDefaultFormulaTest`, `ResolveHookDamageIsAppliedTest`, `ResolveHookMissSpendsApWithoutDamageTest`, `ResolveHookEventPayloadTest`, `ResolveHookKillTriggersDeathTest`, `ResolveHookNegativeDamageIsClampedTest`, `ResolveHookNotCalledWhenAttackInvalidTest`, `PreviewAttackAppliesNothingTest`, `PreviewAttackMatchesRealAttackTest`, `PreviewAttackInvalidInputsTest`, `ResolveHookMayReadQueriesTest`.
