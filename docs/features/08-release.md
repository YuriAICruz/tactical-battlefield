# 08 Release: tests, docs, install

Run after features 01 to 07 land. This is what unblocks the game project.

## Tasks
- [x] Run the full `BattlefieldTestRunner`; fix regressions. Update the test count in `testing.md` (currently claimed 166, unverified).
- [x] Update `architecture.md` (new classes: `FWeaponDef`, `FActionDef`, `FAttackResult`, attributes, tile tags, wait state) and the ownership and lifetime notes for callbacks.
- [x] Extend `architecture.puml` with a class diagram of the new extension points; embed it from `architecture.md`.
- [x] Update `gameplay-rules.md`: new rules, remove fixed quirks from section 10.
- [x] Add `docs/extension-points.md`: how a client registers weapons, actions and the resolve hook, with a short C++ example and the "hooks must not mutate the battle" rule.
- [x] Bump the version in the root `CMakeLists.txt` (minor bump; `SameMajorVersion` compatibility stays).
- [x] `Generate.bat install Debug` and `Generate.bat install Release` into the same `dist` prefix, so the game can use either configuration.
- [x] Verify `BattlefieldGame` still builds and runs its existing sample against the new DLL.

## Done when
`dist/` has Debug and Release builds with the new headers, all tests pass, and the game project can start consuming the new APIs.

## Notes
- Verified (Debug and Release): `BattlefieldTestRunner.exe --quiet` gives `243 tests, 243 passed, 0 failed (693 checks)`. `testing.md` now lists per-suite counts that add up to 243 (the old "166" claim is only the transpiled subset).
- `architecture.puml` has a second diagram (extension points); `architecture.md` embeds it and documents callback ownership and lifetime. New `extension-points.md` covers weapons, the resolve hook, actions and the "hooks must not mutate the battle" rule. `gameplay-rules.md` section 10 had no already-fixed quirks left to remove; a new quirk about `CanAttack` was added.
- Version bumped `1.0.0` to `1.1.0` (`SameMajorVersion` compatibility unchanged, so a game asking for `1.0` still accepts it).
- Found while installing both configurations: Debug and Release both wrote `Battlefield.dll`/`Battlefield.lib`, so the second install silently overwrote the first (the `dist` Debug DLL was byte-identical to Release). Fixed with `DEBUG_POSTFIX d` on the target: Debug installs as `Battlefieldd.dll`/`Battlefieldd.lib`, and the generated `BattlefieldTargets-debug.cmake` points at them.
- `BattlefieldGame` (sibling repo, untouched): `Generate.bat build Debug` and `build Release` succeed against the new `dist`, and the sample starts, moves and ends turns with each configuration's own DLL copied next to the exe.
