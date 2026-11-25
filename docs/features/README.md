# Features: engine support for a Fire Emblem-style game

Goal: let a client (see `BattlefieldGame/docs/features`) build a tactics game **without any game design in C++**. The DLL provides generic mechanics and extension points. It must never know what a "sword", "lance" or "level" is. All of that is data and Lua on the game side.

Rules for every task here:
- Public headers live in `Battlefield/Public`, sources in `Battlefield/Private`.
- STL in exported signatures is the existing convention (same compiler and dynamic CRT, see `building.md`).
- Every feature adds tests in `BattlefieldTests/Private/Tests` and updates the docs it touches.
- Keep the default behaviour of existing APIs unless a task says otherwise, so the current sample game keeps working.

## Order and dependencies

| # | Feature | Depends on | Size |
|---|---------|------------|------|
| 01 | [Bug fixes that block the game](01-bugfixes.md) | none | S |
| 02 | [Unit attribute bag](02-unit-attributes.md) | none | S |
| 03 | [Tile tags](03-tile-tags.md) | none | S |
| 04 | [Unit turn state (acted/waited)](04-unit-turn-state.md) | none | S |
| 05 | [Weapon definitions and range bands](05-weapon-definitions.md) | 02 | M |
| 06 | [Attack resolve hook](06-attack-resolve-hook.md) | 05 | M |
| 07 | [Custom action registry](07-action-registry.md) | 04, 05 | M |
| 08 | [Release: tests, docs, install](08-release.md) | all | S |

01 to 04 are independent and can be done in any order. The game project can start its non-DLL work (Lua host, terminal, renderer) in parallel; it needs 08 before it can use the new APIs.

Size: S = under half a day, M = about a day.

## Out of scope for the DLL (stays in Lua on the game side)
Weapon triangle, hit/crit/avoid formulas, counterattacks, follow-up attacks, XP and levels, growths, classes, terrain names and bonuses, enemy AI, objectives, unit and map data.
