# Gameplay rules implemented by the core

This is a description of what the code *does* today (it is derived from the implementation and verified by the
tests), including placeholders that are not finished yet. Section 10 lists them explicitly.

## 1. Teams and turns

* Teams are bit flags: `Player = 1`, `Enemy = 2`, `Neutral = 4` (`Null = 0` means "no team").
  `AllTeams`, `Invert(mask)` and `HasFlag(mask, flag)` work on masks.
* Order is fixed: **Player → Enemy → Neutral → Player …**. The Player always starts.
* `FBattleController::EndTurn()` raises `OnTurnEnded(current)`, advances the team, increments the turn counter,
  refreshes the **living units of the new team** (see §4) and raises `OnTurnStarted(new)`.
* A unit may act only if it belongs to the current team and can still move **or** do an action
  (`FTeamSystem::CanAct`). The engine never ends a turn by itself; the caller does.

## 2. The grid

* Coordinates are `(x, y)` with `0 ≤ x < width`, `0 ≤ y < height`. Movement is 4-directional (no diagonals).
* A tile has a **weight** (`int8`). Entering a tile costs `1 + weight`. `FGrid::kImpassableWeight` (`-1`, any
  negative value) makes it a wall. The start tile of a search is free.
* A tile is **blocked** when it is impassable *or* occupied. Out-of-range coordinates count as blocked.
* Searches (`GetWeightedNeighbours`, `GetWeightedPath`, `GetWeightedDistance`) take a `checkBlockedTiles` flag:
  `true` → occupied tiles block as well, `false` → only walls block (units can be walked "through", used for
  AI distance estimation). The cost budget is a `uint16`; `GetWeightedDistance` returns `-1` when unreachable
  or when a coordinate is invalid.
* `GetPath` is a separate breadth-first search that ignores weights, always avoids blocked tiles and limits the
  number of steps. `GetNeighbours(coord, out, size, includeCenter)` returns the Manhattan diamond of radius
  `size`, clipped to the board.
* **Line of sight** walks a Bresenham line; every tile strictly between the endpoints must be passable
  (`checkBlockedTiles` decides whether units also block).

## 3. Units

| Group | Fields | Default |
|---|---|---|
| `FCombatStats` | `maxHealth`, `health`, `armor`, `aim`, `dodge` | 10, 10, 0, 10, 0 |
| `FTurnResources` | `ap` (action points), `move` (movement points) | 6, 5 |
| `FCombatWeapon` | `baseDamage`, `baseCost` (AP), `range` | 5, 1, 5 |

* A unit starts at full health with full turn resources and no statuses.
* `ReceiveDamage(n)`: ignored if `n ≤ 0` or the unit is dead; health never drops below 0; raises
  `OnReceiveDamage`, then `OnDeath` exactly once when it reaches 0.
* `ReceiveHeal(n)`: ignored if `n ≤ 0` or the unit is dead; capped at `maxHealth`.
* `SpendActionPoints` / `SpendMovementPoints(n)` fail (return `false`, change nothing) when `n ≤ 0` or the unit has
  fewer than `n`.
* `GetVisionRange()` is the unit's *current* movement points.

## 4. Turn refresh and status effects

`ResetTurnStats()` is called when the unit is created and at the start of its team's turn:

1. `move` and `ap` are set back to their maximum.
2. Every status effect on the unit raises `OnStatusTriggered(unit, type)` and then ticks.

A unit carries up to **8** status effects, at most one per `uint8` type id. `AddStatusEffect` returns `nullptr`
(and raises nothing) for a duplicate or when all slots are used; `RemoveStatusEffect` raises `OnStatusRemoved`.
Statuses have no duration — they stay until removed.

The built-in `FStatusEffect` behaves like a **stun**: on each tick it spends all of the unit's AP and movement, so
the unit cannot do anything that turn. Because the tick runs after the refresh, the refresh is effectively
cancelled. Its tick counter is readable through `GetTicksApplied()`.

## 5. Movement

`MoveUnit(unit, destination)` succeeds only when:

1. the unit's team is the current team and the unit can still move or act,
2. a path exists (occupied tiles and walls block, weights add cost), cost `c ≥ 0`,
3. the unit is alive and has at least `c` movement points (and at least 1 for the grid step itself),
4. the destination tile is free (or already the unit's own tile).

The cost is computed with `passThroughAllies`: tiles held by **allies** can be crossed (they still cost movement)
but never stopped on, so a unit can reach a free tile behind a friend. Enemies always block. A destination holding any
unit costs `-1`.

On success the unit is relocated, `c` movement points are spent and `OnUnitMoved{Unit, From, To}` is raised
(`Unit` is the mover, `From` the tile it left, `To` the destination).
`MoveUnitRaw` skips the path computation and uses a caller-supplied cost.
`FBattleQueries::GetMovementArea` returns every tile reachable with the unit's current movement points,
including its own tile. The optional `passThroughAllies` argument (default `false`) lets the search cross tiles held by
allied units; those tiles are crossed but never listed. `FBaseUnit::GetMovementCost` and
`GetWeightedNeighbours`/`GetWeightedDistance` on `FGrid` take the same option (as a pass-through team).

## 6. Attacking

`AttackUnitWithWeapon(attacker, target)` succeeds only when:

1. both pointers are non-null and the units are **hostile** (different teams, neither `Null`),
2. it is the attacker's team's turn and the attacker can act,
3. the attacker has at least `weapon.baseCost` AP,
4. the Manhattan distance between them is `≤ weapon.range`.

Damage: `distance = max(1, manhattan)`, `multiplier = min(1, aim / distance)` using **integer** division,
`damage = baseDamage × multiplier`. So a shot is full damage while `distance ≤ aim` and **zero** beyond that.

Order of events for a successful attack: `OnUnitDamaged` → (if lethal) the tile is cleared, `OnUnitDied`, and when a
team is wiped out `OnWinnerDeclared` then `OnBattleEnded` → AP is spent → `OnUnitDidAttacked`.

`AttackUnitRaw(attacker, target, range, cost, damage)` and `HealUnitRaw(...)` do the same with caller-supplied
numbers (scripted/ability effects). `HealUnitRaw` requires a **friendly** (non-hostile) target; healing an enemy is
rejected and spends nothing. `DoUnitAction(unit)` requires the unit to be on the current team, spends the unit's action
cost (currently always 1 AP) and raises `OnUnitActionPerformed`.

## 7. Victory

* A team is **defeated** when it has no living unit (a team that never had units is defeated too).
* `HasWinner()` is true when the Player **or** the Enemy team is defeated; Neutral is ignored.
* `GetWinner()` is `Player` if the Enemy is defeated and the Player is not, `Null` if both are defeated (mutual
  destruction), otherwise `Enemy`.
* The controller raises `OnWinnerDeclared` and `OnBattleEnded` from the moment a death produces a winner.

## 8. Starting a battle

`StartBattle(FBattleStartData)` clears the troupes **and every unit from the grid** (so the same `FGrid` can be reused
for a new battle; tile weights are kept) and spawns every `FUnitData` (name, team, stats, resources,
weapon, tile). It fails (returns `false`, no `OnBattleStarted`) as soon as a unit has team `Null`/`Length`, its tile
is blocked/occupied/off-board, or creation fails. `SpawnUnit` can also be used on its own.

## 9. Queries (`FBattleQueries`)

| Query | Meaning |
|---|---|
| `GetUnitsForTeam(mask, out)` / `GetUnitsForCurrentTeam` | living units of the masked teams |
| `GetUnitAt(coord)` | unit on a tile or `nullptr` |
| `GetAttackableTargets(unit, out)` | occupants within weapon range whose team is not the unit's own |
| `GetVisibleUnits(unit, out, mask)` | units of other teams inside the weighted vision area |
| `HasLineOfSight(unit, coord)` | within vision range and not blocked by walls |
| `FindNearestUnit(unit, range)` | nearest living unit of another team inside the Manhattan radius `range`, ranked by weighted distance |
| `GetPath` / `GetPathTo` | A\* path ignoring unit blockers |
| `GetUnitMovementCost` | cost to reach a tile |
| `GetValidActions(unit, cost)` | flags `Move`, `Attack`, `DoActions`; `None` for dead units |
| `HasWinner` / `GetWinner` / `GetCurrentTeam` | battle state |

## 10. Known placeholders and quirks (kept from the Unreal module)

These behaviours were transpiled as-is; they are documented rather than silently "fixed" so the DLL matches the
original game logic. Candidates for a future clean-up (the other quirks of the original were fixed, see `features/01-bugfixes.md`):

1. `armor` and `dodge` are stored but not used by any formula.
2. `ApplyStatus` / `RemoveStatus` check the attacker's **AP** but spend **movement** points.
3. `GetVisibleUnits` computes a line-of-sight list first but then overwrites it with the weighted-area result, so
   effectively only the area test decides.
