# 06 Attack resolve hook

Hit chance, crits, weapon triangle, counters and double attacks are game design. The DLL lets the client decide the outcome of an attack, then applies it through the normal event path.

## Tasks
- [ ] New `FAttackResult` in `Public/Events/`: `int32 damage`, `bool hit`, `bool critical`, `int32 userValue` (opaque).
- [ ] New callback on `FBattleController`: `std::function<FAttackResult(FBaseUnit& attacker, FBaseUnit& target, const FWeaponDef& weapon)> OnResolveAttack`. When unset, the existing distance and aim formula is used.
- [ ] `AttackUnitWithWeapon` calls the hook, then applies damage via the existing `AttackUnitRaw` internals so `OnUnitDidAttacked`, `OnUnitDamaged` and `OnUnitDied` fire as before.
- [ ] Include `hit` and `critical` and `userValue` in `FBattleAttackEvent` so the UI can show "Miss" or "Critical".
- [ ] A miss spends AP, fires the attack event with damage 0, and does not fire `OnUnitDamaged`.
- [ ] Re-entrancy rule: the hook may call read-only queries but not mutate the battle. Document it, and assert in debug builds.
- [ ] `FBattleController::PreviewAttack(attacker, target)` returns the `FAttackResult` from the hook without applying it. Used for the battle forecast box. The hook must be pure for previews (no RNG consumption in the DLL; RNG belongs to the client).
- [ ] Tests: default formula unchanged, hook damage applied, miss path, event payload, kill triggers death event.

## Done when
A client can set one callback and fully own how damage, hit and crit are computed, with events and death handling still working.

## Files
`Public/Core/BattleController.h/.cpp`, `Public/Events/BattleAttackEvent.h`, new `Public/Events/AttackResult.h`, `BattlefieldControllerTests.cpp`.
