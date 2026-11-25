# 08 Release: tests, docs, install

Run after features 01 to 07 land. This is what unblocks the game project.

## Tasks
- [ ] Run the full `BattlefieldTestRunner`; fix regressions. Update the test count in `testing.md` (currently claimed 166, unverified).
- [ ] Update `architecture.md` (new classes: `FWeaponDef`, `FActionDef`, `FAttackResult`, attributes, tile tags, wait state) and the ownership and lifetime notes for callbacks.
- [ ] Extend `architecture.puml` with a class diagram of the new extension points; embed it from `architecture.md`.
- [ ] Update `gameplay-rules.md`: new rules, remove fixed quirks from section 10.
- [ ] Add `docs/extension-points.md`: how a client registers weapons, actions and the resolve hook, with a short C++ example and the "hooks must not mutate the battle" rule.
- [ ] Bump the version in the root `CMakeLists.txt` (minor bump; `SameMajorVersion` compatibility stays).
- [ ] `Generate.bat install Debug` and `Generate.bat install Release` into the same `dist` prefix, so the game can use either configuration.
- [ ] Verify `BattlefieldGame` still builds and runs its existing sample against the new DLL.

## Done when
`dist/` has Debug and Release builds with the new headers, all tests pass, and the game project can start consuming the new APIs.
