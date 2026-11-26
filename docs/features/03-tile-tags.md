# 03 Tile tags

Tiles only have a movement `weight`. The game needs to know terrain kind (forest, fort...) to apply defence and avoid bonuses in Lua. The DLL stores an opaque integer tag.

## Tasks
- [x] Add `int32 tag = 0` to `FTileData` (`Public/Core/Grid.h`).
- [x] API: `FGrid::SetTileTag(x, y, tag)` and `FGrid::GetTileTag(x, y)` (returns 0 for out-of-bounds).
- [x] Add `SetTile(x, y, weight, tag)` convenience for map loading.
- [x] Make sure tags do not affect pathing, blocking or line of sight.
- [x] Tests in `GridTests.cpp`: default is 0, set/get, out-of-bounds, pathing unchanged.

## Done when
A client can paint tile tags and read them back by coordinate; all existing grid tests still pass.

## Notes
- `SetTileTag` and `SetTile` ignore out-of-bounds coordinates; `GetTileTag` returns 0 there.
- Tags are stored on `FTileData::tag` and are never read by `IsBlocked`, the search routines or `HasLineOfSight`.
- Tests: `TileTagDefaultIsZeroTest`, `TileTagSetGetTest`, `TileTagOutOfBoundsTest`, `TileSetWeightAndTagTest`, `TileTagDoesNotAffectPathingTest`, `TileTagDoesNotAffectBlockingOrSightTest`.

## Files
`Public/Core/Grid.h`, `Private/Core/Grid.cpp`, `BattlefieldTests/Private/Tests/GridTests.cpp`.
