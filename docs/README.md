# BattlefieldCpp — Project Description

BattlefieldCpp is the **pure C++20, engine-independent version of the Hellmarch `Battlefield` module**.
It contains the rules of a turn-based, grid-based tactical battle (think XCOM / Fire Emblem style):
units on a square grid, three teams that act in rotation, movement budgets, action points,
weapon attacks, status effects, line of sight and win detection.

The original module lives in `Source/Battlefield` and is built by Unreal Build Tool. Its core logic
never needed the engine (it only used `uint8`/`int32`, `UE_LOG`, `FName` and a few math types), so it
has been transpiled into two plain CMake projects that compile to Windows DLLs (and to shared
libraries on other platforms) without Unreal installed.

## The two projects

| Project | Output | What it is |
|---|---|---|
| [`Battlefield/`](../Battlefield) | `Battlefield.dll` | The game core: grid, units, teams, battle controller, queries, events. |
| [`BattlefieldTests/`](../BattlefieldTests) | `BattlefieldTests.dll` (+ `BattlefieldTestRunner.exe`) | The 166 automation tests transpiled from `Source/Battlefield/Private/Tests`, exposed through a small C API, plus a console host that runs them. |

```
BattlefieldCpp/
├─ CMakeLists.txt              root build (adds both projects)
├─ docs/                       this documentation
├─ Battlefield/                PROJECT 1 – the DLL
│  ├─ Public/                  headers shipped to clients
│  │  ├─ Platform/             CoreTypes.h (int32/uint8…, BATTLEFIELD_API), BattlefieldLog.h
│  │  ├─ Core/                 Grid, BaseUnit, BattleController, BattleQueries, TeamSystem, Troupe…
│  │  ├─ Enums/                ETeams, EBattleAction
│  │  ├─ Events/               event payload structs
│  │  └─ Math/                 FVector, FUintVector, FMath, FGridMath
│  └─ Private/                 implementation files
└─ BattlefieldTests/           PROJECT 2 – the tests DLL
   ├─ Public/BattlefieldTests.h   C API of the DLL
   ├─ Private/Framework/          automation-test shim + exported entry points
   ├─ Private/Tests/              Actor, Team, Grid, Controller, Query and Flow tests
   └─ Runner/main.cpp             console host for the DLL
```

## What the core does

* **Grid** – `FGrid` is a width × height board with 4-directional movement. Every tile has an integer
  *weight* (extra movement cost, `-1` = impassable) and at most one occupying entity. It offers
  breadth-first paths, weighted (Dijkstra / A\*) paths and distances, reachable-area queries, Manhattan
  neighbourhoods, Bresenham line of sight and visible-tile sets.
* **Units** – `FBaseUnit` / `FUnit` carry combat stats (health, armor, aim, dodge), turn resources
  (action points, movement points), a weapon (damage, AP cost, range) and up to eight status effects.
* **Teams and turns** – `FTeamSystem` rotates `Player → Enemy → Neutral → Player…`. Only units of the
  current team may act; at the start of their team's turn their resources refresh and their status
  effects tick.
* **Battle controller** – `FBattleController` is the single entry point that game code (UI, AI) calls:
  spawn units, move, attack, heal, apply/remove statuses, perform an action, start a battle, end a turn.
  Every successful operation raises a callback (`OnUnitMoved`, `OnUnitDamaged`, `OnUnitDied`,
  `OnWinnerDeclared`, …) so presentation layers can stay decoupled from the rules.
* **Queries** – `FBattleQueries` is a read-only façade: movement area, attackable targets, visible
  units, line of sight, nearest enemy, paths, valid actions, winner.

See [architecture.md](architecture.md) for the class-by-class design and
[gameplay-rules.md](gameplay-rules.md) for the exact rules the code enforces.

## Quick start

```powershell
cd BattlefieldCpp
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
build\bin\Release\BattlefieldTestRunner.exe
```

Using the library from your own code:

```cpp
#include "Core/BattleController.h"

FGrid grid(10, 10);
FBattleController controller(&grid);

FBaseUnit::FCombatStats stats{10, 10, 0, 10, 0};   // maxHealth, health, armor, aim, dodge
FBaseUnit::FTurnResources turn{5, 6};              // ap, move
FBaseUnit::FCombatWeapon rifle{5, 1, 5};           // damage, ap cost, range

FBaseUnit* soldier = controller.SpawnUnit(ETeams::Player, stats, turn, rifle, {1, 1});
FBaseUnit* raider  = controller.SpawnUnit(ETeams::Enemy,  stats, turn, rifle, {2, 1});

controller.OnUnitDied = [](const FBaseUnit* unit) { /* play animation */ };
controller.AttackUnitWithWeapon(soldier, raider);  // true when the attack was legal
controller.EndTurn();                              // Enemy team's turn now
```

Link against `Battlefield` (CMake target) – it brings the include path and the
`BATTLEFIELD_API` import macro with it. Further reading:

* [building.md](building.md) – requirements, configure/build/install, other compilers
* [testing.md](testing.md) – the test DLL, its C API, the runner, how to add tests
* [unreal-port-notes.md](unreal-port-notes.md) – what changed versus the Unreal module, and known quirks
