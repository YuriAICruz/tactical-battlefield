#include "Framework/AutomationShim.h"
#include "Math/GridMath.h"
#include "Core/Grid.h"
#include "Core/BaseUnit.h"
#include "Tests/TestsHelper.h"

static constexpr float kTileSize = 100.f;
const FUintVector kNoOffset(0, 0, 0);

constexpr int32 kManhattanDistance = 10;

#define GRID_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Grid." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )

GRID_TEST(CreationTest)

bool CreationTest::RunTest(const FString& Parameters)
{
	const FGrid Grid = TestsHelper::CreateTestGrid();
	TestEqual(TEXT("Grid width should be 10"), Grid.GetWidth(), 10);
	TestEqual(TEXT("Grid height should be 10"), Grid.GetHeight(), 10);
	return true;
}

GRID_TEST(BoundsTest)

bool BoundsTest::RunTest(const FString& Parameters)
{
	const FGrid Grid = TestsHelper::CreateTestGrid();

	TestTrue(TEXT("Valid inside grid"), Grid.IsValid(5, 5));
	TestTrue(TEXT("Origin valid"), Grid.IsValid(0, 0));

	TestFalse(TEXT("Negative X invalid"), Grid.IsValid(-1, 5));
	TestFalse(TEXT("Negative Y invalid"), Grid.IsValid(5, -1));
	TestFalse(TEXT("Out of bounds X"), Grid.IsValid(100, 5));
	TestFalse(TEXT("Out of bounds Y"), Grid.IsValid(5, 100));

	return true;
}

GRID_TEST(TileAccessTest)

bool TileAccessTest::RunTest(const FString& Parameters)
{
	const FGrid Grid = TestsHelper::CreateTestGrid();

	const FGrid::FTileData* Tile = Grid.TryGetTile(3, 3);

	TestNotNull(TEXT("Tile should exist"), Tile);

	if (!Tile)
	{
		return false;
	}

	TestEqual(TEXT("Tile X should match"), Tile->coord.x, 3);
	TestEqual(TEXT("Tile Y should match"), Tile->coord.y, 3);

	TestNull(TEXT("Invalid tile should return null"), Grid.TryGetTile(-1, -1));

	return true;
}

GRID_TEST(OccupancyTest)

bool OccupancyTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	const FGrid::FTileData* tile = Grid.TryGetTile(2, 2);
	TestNotNull(TEXT("Tile should exist"), tile);

	if (!tile)
	{
		return false;
	}

	TestNull(TEXT("Tile should start empty"), tile->unit);

	FGridEntity dummyOccupant;
	Grid.MoveUnitToTile(tile->coord, &dummyOccupant);
	// Get again to make sure it is still the same pointer.
	tile = Grid.TryGetTile(2, 2);

	TestNotNull(TEXT("Tile should now be occupied"), tile->unit);
	TestEqual(TEXT("Occupant pointer should match"), *tile->unit, dummyOccupant);

	return true;
}

GRID_TEST(ConversionBasicTest)

bool ConversionBasicTest::RunTest(const FString& Parameters)
{
	FGrid::FGridCoordinate coord(2, 3);

	FVector worldPosition = FGridMath::GridToWorld(coord, kTileSize, kNoOffset);

	TestTrue(TEXT("World X"), FMath::IsNearlyEqual(worldPosition.X, coord.x * kTileSize));
	TestTrue(TEXT("World Y"), FMath::IsNearlyEqual(worldPosition.Y, coord.y * kTileSize));

	FGrid::FGridCoordinate coordinate = FGridMath::WorldToGrid(worldPosition, kTileSize, kNoOffset);

	TestEqual(TEXT("Back X"), coordinate.x, coord.x);
	TestEqual(TEXT("Back Y"), coordinate.y, coord.y);

	return true;
}

GRID_TEST(GridToWorldOriginTest)

bool GridToWorldOriginTest::RunTest(const FString& Parameters)
{
	FVector Position =
		FGridMath::GridToWorld(
			0,
			0,
			kTileSize,
			kNoOffset);

	TestEqual(TEXT("X"), Position.X, 0.0);
	TestEqual(TEXT("Y"), Position.Y, 0.0);

	return true;
}

GRID_TEST(GridToWorldCoordinateTest)

bool GridToWorldCoordinateTest::RunTest(const FString& Parameters)
{
	FVector Position =
		FGridMath::GridToWorld(
			2,
			3,
			kTileSize,
			kNoOffset);

	TestEqual(TEXT("X"), Position.X, 200.0);
	TestEqual(TEXT("Y"), Position.Y, 300.0);

	return true;
}

GRID_TEST(GridToWorldWithOffsetTest)

bool GridToWorldWithOffsetTest::RunTest(const FString& Parameters)
{
	FVector Position =
		FGridMath::GridToWorld(
			2,
			3,
			100.f,
			FUintVector(50, 75, 0));

	TestEqual(TEXT("X"), Position.X, 250.0);
	TestEqual(TEXT("Y"), Position.Y, 375.0);

	return true;
}

GRID_TEST(WorldToGridOriginTest)

bool WorldToGridOriginTest::RunTest(const FString& Parameters)
{
	auto Coord =
		FGridMath::WorldToGrid(
			FVector(0.f, 0.f, 0.f),
			100.f,
			kNoOffset);

	TestEqual(TEXT("X"), Coord.x, 0);
	TestEqual(TEXT("Y"), Coord.y, 0);

	return true;
}

GRID_TEST(WorldToGridFlooringTest)

bool WorldToGridFlooringTest::RunTest(const FString& Parameters)
{
	auto Coord =
		FGridMath::WorldToGrid(
			FVector(299.f, 199.f, 0.f),
			100.f,
			kNoOffset);

	TestEqual(TEXT("X"), Coord.x, 2);
	TestEqual(TEXT("Y"), Coord.y, 1);

	return true;
}

GRID_TEST(WorldToGridOffsetTest)

bool WorldToGridOffsetTest::RunTest(const FString& Parameters)
{
	auto Coord =
		FGridMath::WorldToGrid(
			FVector(250.f, 375.f, 0.f),
			100.f,
			FUintVector(50, 75, 0));

	TestEqual(TEXT("X"), Coord.x, 2);
	TestEqual(TEXT("Y"), Coord.y, 3);

	return true;
}

GRID_TEST(RoundTripGridWorldGridTest)

bool RoundTripGridWorldGridTest::RunTest(const FString& Parameters)
{
	FGrid::FGridCoordinate Original(7, 4);

	FVector World =
		FGridMath::GridToWorld(
			Original,
			100.f,
			kNoOffset);

	auto Result =
		FGridMath::WorldToGrid(
			World,
			100.f,
			kNoOffset);

	TestEqual(TEXT("X"), Result.x, Original.x);
	TestEqual(TEXT("Y"), Result.y, Original.y);

	return true;
}

GRID_TEST(RoundTripGridWorldGridOffsetTest)

bool RoundTripGridWorldGridOffsetTest::RunTest(const FString& Parameters)
{
	FGrid::FGridCoordinate Original(7, 4);

	FUintVector Offset(500, 250, 0);

	FVector World =
		FGridMath::GridToWorld(
			Original,
			100.f,
			Offset);

	auto Result =
		FGridMath::WorldToGrid(
			World,
			100.f,
			Offset);

	TestEqual(TEXT("X"), Result.x, Original.x);
	TestEqual(TEXT("Y"), Result.y, Original.y);

	return true;
}

GRID_TEST(GridToWorldCenteredOriginTest)

bool GridToWorldCenteredOriginTest::RunTest(const FString& Parameters)
{
	FVector Position =
		FGridMath::GridToWorldCentered(
			0,
			0,
			100.f,
			10,
			10,
			kNoOffset);

	TestEqual(TEXT("X"), Position.X, -500.0);
	TestEqual(TEXT("Y"), Position.Y, -500.0);

	return true;
}

GRID_TEST(GridToWorldCenteredMiddleTest)

bool GridToWorldCenteredMiddleTest::RunTest(const FString& Parameters)
{
	FVector Position =
		FGridMath::GridToWorldCentered(
			5,
			5,
			100.f,
			10,
			10,
			kNoOffset);

	TestEqual(TEXT("X"), Position.X, 0.0);
	TestEqual(TEXT("Y"), Position.Y, 0.0);

	return true;
}

GRID_TEST(CenteredRoundTripTest)

bool CenteredRoundTripTest::RunTest(const FString& Parameters)
{
	FGrid::FGridCoordinate Original(3, 7);

	FVector World =
		FGridMath::GridToWorldCentered(
			Original,
			100.f,
			10,
			10,
			kNoOffset);

	auto Result =
		FGridMath::WorldCenteredToGrid(
			World,
			100.f,
			10,
			10,
			kNoOffset);

	TestEqual(TEXT("X"), Result.x, Original.x);
	TestEqual(TEXT("Y"), Result.y, Original.y);

	return true;
}

GRID_TEST(WorldToGridNegativeTest)

bool WorldToGridNegativeTest::RunTest(const FString& Parameters)
{
	auto Coord =
		FGridMath::WorldToGrid(
			FVector(-1.f, -1.f, 0.f),
			100.f,
			kNoOffset);

	TestEqual(TEXT("X"), Coord.x, -1);
	TestEqual(TEXT("Y"), Coord.y, -1);

	return true;
}

GRID_TEST(NeighbourTest)

bool NeighbourTest::RunTest(const FString& Parameters)
{
	const FGrid grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> neighbours; //TArray
	grid.GetNeighbours(FGrid::FGridCoordinate(5, 5), neighbours);

	TestEqual(TEXT("Should have 4 neighbours"), static_cast<int32>(neighbours.size()), 4);

	bool bHasUp = std::ranges::find(neighbours, FGrid::FGridCoordinate(5, 6)) != neighbours.end();
	bool bHasDown = std::ranges::find(neighbours, FGrid::FGridCoordinate(5, 4)) != neighbours.end();
	bool bHasLeft = std::ranges::find(neighbours, FGrid::FGridCoordinate(4, 5)) != neighbours.end();
	bool bHasRight = std::ranges::find(neighbours, FGrid::FGridCoordinate(6, 5)) != neighbours.end();

	TestTrue(TEXT("Has up"), bHasUp);
	TestTrue(TEXT("Has down"), bHasDown);
	TestTrue(TEXT("Has left"), bHasLeft);
	TestTrue(TEXT("Has right"), bHasRight);

	grid.GetNeighbours(FGrid::FGridCoordinate(0, 0), neighbours, 1, true);
	TestEqual(TEXT("Should have 2 neighbours and the center"), static_cast<int32>(neighbours.size()), 3);

	grid.GetNeighbours(FGrid::FGridCoordinate(5, 5), neighbours, 2, true);
	TestEqual(TEXT("Should have 12 neighbours and the center"), static_cast<int32>(neighbours.size()), 13);

	grid.GetNeighbours(FGrid::FGridCoordinate(5, 5), neighbours, 3, false);
	TestEqual(TEXT("Should have 24"), static_cast<int32>(neighbours.size()), 24);

	bHasUp = std::ranges::find(neighbours, FGrid::FGridCoordinate(5, 7)) != neighbours.end();
	bHasDown = std::ranges::find(neighbours, FGrid::FGridCoordinate(5, 2)) != neighbours.end();
	bHasLeft = std::ranges::find(neighbours, FGrid::FGridCoordinate(8, 5)) != neighbours.end();
	bHasRight = std::ranges::find(neighbours, FGrid::FGridCoordinate(6, 6)) != neighbours.end();

	TestTrue(TEXT("Has up"), bHasUp);
	TestTrue(TEXT("Has down"), bHasDown);
	TestTrue(TEXT("Has left"), bHasLeft);
	TestTrue(TEXT("Has right"), bHasRight);

	return true;
}

GRID_TEST(PathSameTileTest)

bool PathSameTileTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({5, 5}, {5, 5}, 5, Path);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path should contain one node"), static_cast<int32>(Path.size()), 1);

	if (Path.size() == 1)
	{
		TestEqual(TEXT("X"), Path[0].x, 5);
		TestEqual(TEXT("Y"), Path[0].y, 5);
	}

	return true;
}

GRID_TEST(PathHorizontalTest)

bool PathHorizontalTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({1, 1}, {4, 1}, 5, Path);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path length"), static_cast<int32>(Path.size()), 4);
	if (Path.size() == 4)
	{
		TestEqual(TEXT("Start X"), Path.front().x, 1);
		TestEqual(TEXT("End X"), Path.back().x, 4);
	}

	return true;
}

GRID_TEST(PathVerticalTest)

bool PathVerticalTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({3, 2}, {3, 6}, 6, Path);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path length"), static_cast<int32>(Path.size()), 5);
	return true;
}

GRID_TEST(PathAdjacentTest)

bool PathAdjacentTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({5, 5}, {6, 5}, 5, Path);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path should contain start and end"), static_cast<int32>(Path.size()), 2);
	return true;
}

GRID_TEST(PathInvalidStartTest)

bool PathInvalidStartTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({-1, 0}, {5, 5}, 3, Path);
	TestFalse(TEXT("Path should fail"), bFound);
	TestEqual(TEXT("Path should be empty"), static_cast<int32>(Path.size()), 0);
	return true;
}

GRID_TEST(PathInvalidEndTest)

bool PathInvalidEndTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({5, 5}, {50, 50}, 50, Path);
	TestFalse(TEXT("Path should fail"), bFound);
	return true;
}

GRID_TEST(PathBlockedDestinationTest)

bool PathBlockedDestinationTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	FGridEntity Occupant;
	Grid.MoveUnitToTile({5, 5}, &Occupant);

	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({1, 1}, {5, 5}, 10, Path);

	TestFalse(TEXT("Path should fail"), bFound);
	return true;
}

GRID_TEST(PathAroundObstacleTest)

bool PathAroundObstacleTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	FGridEntity Blocker;

	Grid.MoveUnitToTile({2, 1}, &Blocker);

	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({1, 1}, {3, 1}, 10, Path);
	TestTrue(TEXT("Path should exist"), bFound);

	bool bContainsBlockedTile = std::ranges::find(Path, FGrid::FGridCoordinate(2, 1)) != Path.end();
	TestFalse(TEXT("Path should avoid blocker"), bContainsBlockedTile);
	return true;
}

GRID_TEST(PathImpossibleTest)

bool PathImpossibleTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	FGridEntity A;
	FGridEntity B;
	FGridEntity C;

	Grid.MoveUnitToTile({1, 0}, &A);
	Grid.MoveUnitToTile({1, 1}, &B);
	Grid.MoveUnitToTile({1, 2}, &C);

	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetPath({0, 1}, {2, 1}, 3, Path);

	TestFalse(TEXT("Path should not exist"), bFound);

	return true;
}

GRID_TEST(PathOptimalLengthTest)

bool PathOptimalLengthTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> Path;

	const bool bFound = Grid.GetPath({0, 0}, {5, 5}, kManhattanDistance + 5, Path);
	TestTrue(TEXT("Path should exist"), bFound);

	TestEqual(TEXT("Shortest path length"), static_cast<int32>(Path.size()), kManhattanDistance + 1);

	return true;
}

GRID_TEST(WeightedPathSameTileTest)

bool WeightedPathSameTileTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({5, 5}, {5, 5}, Path, 5, true);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path should contain one node"), static_cast<int32>(Path.size()), 1);

	if (Path.size() == 1)
	{
		TestEqual(TEXT("X"), Path[0].x, 5);
		TestEqual(TEXT("Y"), Path[0].y, 5);
	}

	return true;
}

GRID_TEST(WeightedPathHorizontalTest)

bool WeightedPathHorizontalTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({1, 1}, {4, 1}, Path, 5, true);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path length"), static_cast<int32>(Path.size()), 4);
	if (Path.size() == 4)
	{
		TestEqual(TEXT("Start X"), Path.front().x, 1);
		TestEqual(TEXT("End X"), Path.back().x, 4);
	}

	return true;
}

GRID_TEST(WeightedPathVerticalTest)

bool WeightedPathVerticalTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({3, 2}, {3, 6}, Path, 6, true);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path length"), static_cast<int32>(Path.size()), 5);
	return true;
}

GRID_TEST(WeightedPathAdjacentTest)

bool WeightedPathAdjacentTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({5, 5}, {6, 5}, Path, 5, true);
	TestTrue(TEXT("Path should exist"), bFound);
	TestEqual(TEXT("Path should contain start and end"), static_cast<int32>(Path.size()), 2);
	return true;
}

GRID_TEST(WeightedPathInvalidStartTest)

bool WeightedPathInvalidStartTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({-1, 0}, {5, 5}, Path, 3, true);
	TestFalse(TEXT("Path should fail"), bFound);
	TestEqual(TEXT("Path should be empty"), static_cast<int32>(Path.size()), 0);
	return true;
}

GRID_TEST(WeightedPathInvalidEndTest)

bool WeightedPathInvalidEndTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({5, 5}, {50, 50}, Path, 50, true);
	TestFalse(TEXT("Path should fail"), bFound);
	return true;
}

GRID_TEST(WeightedPathBlockedDestinationTest)

bool WeightedPathBlockedDestinationTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	FGridEntity Occupant;
	Grid.MoveUnitToTile({5, 5}, &Occupant);

	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({1, 1}, {5, 5}, Path, 10, true);

	TestFalse(TEXT("Path should fail"), bFound);
	return true;
}

GRID_TEST(WeightedPathAroundObstacleTest)

bool WeightedPathAroundObstacleTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();
	FGridEntity Blocker;

	Grid.MoveUnitToTile({2, 1}, &Blocker);

	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({1, 1}, {3, 1}, Path, 10, true);
	TestTrue(TEXT("Path should exist"), bFound);

	bool bContainsBlockedTile = std::ranges::find(Path, FGrid::FGridCoordinate(2, 1)) != Path.end();
	TestFalse(TEXT("Path should avoid blocker"), bContainsBlockedTile);
	return true;
}

GRID_TEST(WeightedPathImpossibleTest)

bool WeightedPathImpossibleTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	FGridEntity A;
	FGridEntity B;
	FGridEntity C;

	Grid.MoveUnitToTile({1, 0}, &A);
	Grid.MoveUnitToTile({1, 1}, &B);
	Grid.MoveUnitToTile({1, 2}, &C);

	std::vector<FGrid::FGridCoordinate> Path;
	const bool bFound = Grid.GetWeightedPath({0, 1}, {2, 1}, Path, 3, true);

	TestFalse(TEXT("Path should not exist"), bFound);

	return true;
}

GRID_TEST(WeightedPathOptimalLengthTest)

bool WeightedPathOptimalLengthTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> Path;

	const bool bFound = Grid.GetWeightedPath({0, 0}, {5, 5}, Path, kManhattanDistance + 5, true);
	TestTrue(TEXT("Path should exist"), bFound);

	TestEqual(TEXT("Shortest path length"), static_cast<int32>(Path.size()), kManhattanDistance + 1);

	return true;
}

GRID_TEST(WeightedPathPrefersCheaperRoute)

bool WeightedPathPrefersCheaperRoute::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	Grid.SetTileWeight(2, 0, 100);
	std::vector<FGrid::FGridCoordinate> Path;
	bool bFound = Grid.GetWeightedPath(
		{0, 0},
		{3, 0},
		Path,
		200, true);

	TestTrue(TEXT("Path should exist"), bFound);

	bool bUsesExpensiveTile = std::ranges::find(Path, FGrid::FGridCoordinate(2, 0)) != Path.end();
	TestFalse(TEXT("Path should avoid expensive tile"), bUsesExpensiveTile);
	return true;
}

GRID_TEST(WeightedNeighboursRangeZero)

bool WeightedNeighboursRangeZero::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({5, 5}, Tiles, 0, true);

	TestEqual(TEXT("Only start tile"), static_cast<int32>(Tiles.size()), 1);
	TestTrue(TEXT("Contains start"), TestsHelper::ContainsTile(Tiles, {5, 5}));
	return true;
}

GRID_TEST(WeightedNeighboursRangeOne)

bool WeightedNeighboursRangeOne::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({5, 5}, Tiles, 1, true);

	TestEqual(TEXT("Reachable tile count"), static_cast<int32>(Tiles.size()), 5);

	TestTrue(TEXT("Center"), TestsHelper::ContainsTile(Tiles, {5, 5}));
	TestTrue(TEXT("Up"), TestsHelper::ContainsTile(Tiles, {5, 6}));
	TestTrue(TEXT("Down"), TestsHelper::ContainsTile(Tiles, {5, 4}));
	TestTrue(TEXT("Left"), TestsHelper::ContainsTile(Tiles, {4, 5}));
	TestTrue(TEXT("Right"), TestsHelper::ContainsTile(Tiles, {6, 5}));

	return true;
}

GRID_TEST(WeightedNeighboursBlockedTile)

bool WeightedNeighboursBlockedTile::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	FGridEntity Blocker;

	Grid.MoveUnitToTile({6, 5}, &Blocker);

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({5, 5}, Tiles, 1, true);

	TestFalse(TEXT("Blocked tile should not appear"), TestsHelper::ContainsTile(Tiles, {6, 5}));

	return true;
}

GRID_TEST(WeightedNeighboursHeavyTileOutsideRange)

bool WeightedNeighboursHeavyTileOutsideRange::RunTest(const FString& Parameters)
{
	FGrid Grid(2, 1);

	Grid.SetTileWeight(1, 0, 10);

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({0, 0}, Tiles, 5, true);

	TestFalse(TEXT("Heavy tile unreachable"), TestsHelper::ContainsTile(Tiles, {1, 0}));

	return true;
}

GRID_TEST(WeightedNeighboursHeavyTileWithinRange)

bool WeightedNeighboursHeavyTileWithinRange::RunTest(const FString& Parameters)
{
	FGrid Grid(2, 1);

	Grid.SetTileWeight(1, 0, 3);

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({0, 0}, Tiles, 4, true);

	TestTrue(TEXT("Heavy tile reachable"), TestsHelper::ContainsTile(Tiles, {1, 0}));

	return true;
}

GRID_TEST(WeightedNeighboursAlternativeRoute)

bool WeightedNeighboursAlternativeRoute::RunTest(const FString& Parameters)
{
	FGrid Grid(3, 2);

	Grid.SetTileWeight(1, 0, 100);

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({0, 0}, Tiles, 4, true);

	TestTrue(TEXT("Target reachable via alternate path"), TestsHelper::ContainsTile(Tiles, {2, 1}));

	return true;
}

GRID_TEST(WeightedNeighboursCorner)

bool WeightedNeighboursCorner::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({0, 0}, Tiles, 1, true);

	TestEqual(TEXT("Corner reachable count"), static_cast<int32>(Tiles.size()), 3);

	TestTrue(TEXT("Center"), TestsHelper::ContainsTile(Tiles, {0, 0}));
	TestTrue(TEXT("Right"), TestsHelper::ContainsTile(Tiles, {1, 0}));
	TestTrue(TEXT("Up"), TestsHelper::ContainsTile(Tiles, {0, 1}));

	return true;
}

GRID_TEST(WeightedNeighboursRadiusTwo)

bool WeightedNeighboursRadiusTwo::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({5, 5}, Tiles, 2, true);

	TestEqual(TEXT("Reachable count"), static_cast<int32>(Tiles.size()), 13);

	return true;
}

GRID_TEST(WeightedNeighboursSurrounded)

bool WeightedNeighboursSurrounded::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	FGridEntity A;
	FGridEntity B;
	FGridEntity C;
	FGridEntity D;

	Grid.MoveUnitToTile({5, 6}, &A);
	Grid.MoveUnitToTile({5, 4}, &B);
	Grid.MoveUnitToTile({4, 5}, &C);
	Grid.MoveUnitToTile({6, 5}, &D);

	std::vector<FGrid::FGridCoordinate> Tiles;

	Grid.GetWeightedNeighbours({5, 5}, Tiles, 10, true);

	TestEqual(TEXT("Only start tile reachable"), static_cast<int32>(Tiles.size()), 1);
	TestTrue(TEXT("Contains start"), TestsHelper::ContainsTile(Tiles, {5, 5}));
	return true;
}

GRID_TEST(WeightedDistanceSameTileTest)

bool WeightedDistanceSameTileTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Distance to self"), Grid.GetWeightedDistance({5, 5}, {5, 5}, true), 0);

	return true;
}

GRID_TEST(WeightedDistanceAdjacentTest)

bool WeightedDistanceAdjacentTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("One move costs one"), Grid.GetWeightedDistance({5, 5}, {6, 5}, true), 1);

	return true;
}

GRID_TEST(WeightedDistanceHorizontalTest)

bool WeightedDistanceHorizontalTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Three horizontal moves"), Grid.GetWeightedDistance({1, 1}, {4, 1}, true), 3);

	return true;
}

GRID_TEST(WeightedDistanceManhattanTest)

bool WeightedDistanceManhattanTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Open grid distance"), Grid.GetWeightedDistance({0, 0}, {5, 5}, true), 10);

	return true;
}

GRID_TEST(WeightedDistanceInvalidStartTest)

bool WeightedDistanceInvalidStartTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Invalid start"), Grid.GetWeightedDistance({-1, 0}, {5, 5}, true), -1);

	return true;
}

GRID_TEST(WeightedDistanceInvalidDestinationTest)

bool WeightedDistanceInvalidDestinationTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Invalid destination"), Grid.GetWeightedDistance({5, 5}, {100, 100}, true), -1);

	return true;
}

GRID_TEST(WeightedDistanceHeavyTileTest)

bool WeightedDistanceHeavyTileTest::RunTest(const FString& Parameters)
{
	FGrid Grid(2, 1);

	Grid.SetTileWeight(1, 0, 5);

	// 1 move + weight 5
	TestEqual(TEXT("Cost should include tile weight"), Grid.GetWeightedDistance({0, 0}, {1, 0}, true), 6);

	return true;
}

GRID_TEST(WeightedDistancePrefersCheaperRouteTest)

bool WeightedDistancePrefersCheaperRouteTest::RunTest(const FString& Parameters)
{
	FGrid Grid(4, 2);

	Grid.SetTileWeight(1, 0, 100);

	TestEqual(TEXT("Should route around expensive tile"), Grid.GetWeightedDistance({0, 0}, {3, 0}, true), 5);

	return true;
}

GRID_TEST(TileTagDefaultIsZeroTest)

bool TileTagDefaultIsZeroTest::RunTest(const FString& Parameters)
{
	const FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Fresh tile has tag 0"), Grid.GetTileTag(0, 0), 0);
	TestEqual(TEXT("Fresh tile data has tag 0"), Grid.TryGetTile(5, 5)->tag, 0);

	return true;
}

GRID_TEST(TileTagSetGetTest)

bool TileTagSetGetTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	Grid.SetTileTag(3, 4, 7);
	Grid.SetTileTag(9, 9, -2);

	TestEqual(TEXT("Tag read back"), Grid.GetTileTag(3, 4), 7);
	TestEqual(TEXT("Negative tag read back"), Grid.GetTileTag(9, 9), -2);
	TestEqual(TEXT("Tag is exposed on the tile"), Grid.TryGetTile(3, 4)->tag, 7);
	TestEqual(TEXT("Neighbour untouched"), Grid.GetTileTag(4, 3), 0);

	Grid.SetTileTag(3, 4, 0);
	TestEqual(TEXT("Tag can be cleared"), Grid.GetTileTag(3, 4), 0);

	return true;
}

GRID_TEST(TileTagOutOfBoundsTest)

bool TileTagOutOfBoundsTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	TestEqual(TEXT("Negative coords return 0"), Grid.GetTileTag(-1, -1), 0);
	TestEqual(TEXT("Past width returns 0"), Grid.GetTileTag(10, 0), 0);
	TestEqual(TEXT("Past height returns 0"), Grid.GetTileTag(0, 10), 0);

	// Must not crash nor write anywhere.
	Grid.SetTileTag(-1, 0, 5);
	Grid.SetTileTag(10, 10, 5);
	Grid.SetTile(100, 100, 3, 5);

	TestEqual(TEXT("Still 0 out of bounds"), Grid.GetTileTag(10, 10), 0);
	TestEqual(TEXT("In-bounds tiles unaffected"), Grid.GetTileTag(0, 0), 0);

	return true;
}

GRID_TEST(TileSetWeightAndTagTest)

bool TileSetWeightAndTagTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	Grid.SetTile(2, 3, 4, 9);

	const FGrid::FTileData* Tile = Grid.TryGetTile(2, 3);
	TestEqual(TEXT("Weight set"), static_cast<int32>(Tile->weight), 4);
	TestEqual(TEXT("Tag set"), Tile->tag, 9);
	TestEqual(TEXT("Tag via getter"), Grid.GetTileTag(2, 3), 9);

	return true;
}

GRID_TEST(TileTagDoesNotAffectPathingTest)

bool TileTagDoesNotAffectPathingTest::RunTest(const FString& Parameters)
{
	FGrid Plain = TestsHelper::CreateTestGrid();
	FGrid Tagged = TestsHelper::CreateTestGrid();

	// Tag every tile, including a wall of tags across the straight route.
	for (int32 y = 0; y < 10; ++y)
	{
		for (int32 x = 0; x < 10; ++x)
		{
			Tagged.SetTileTag(x, y, 1 + x + y);
		}
	}

	std::vector<FGrid::FGridCoordinate> PlainPath;
	std::vector<FGrid::FGridCoordinate> TaggedPath;
	TestTrue(TEXT("Plain path"), Plain.GetPath({0, 0}, {5, 5}, 20, PlainPath));
	TestTrue(TEXT("Tagged path"), Tagged.GetPath({0, 0}, {5, 5}, 20, TaggedPath));
	TestTrue(TEXT("BFS path identical"), PlainPath == TaggedPath);

	TestTrue(TEXT("Plain weighted path"), Plain.GetWeightedPath({0, 0}, {5, 5}, PlainPath, 20, true));
	TestTrue(TEXT("Tagged weighted path"), Tagged.GetWeightedPath({0, 0}, {5, 5}, TaggedPath, 20, true));
	TestTrue(TEXT("Weighted path identical"), PlainPath == TaggedPath);

	TestEqual(TEXT("Weighted distance unchanged"),
	          Tagged.GetWeightedDistance({0, 0}, {5, 5}, true), Plain.GetWeightedDistance({0, 0}, {5, 5}, true));

	std::vector<FGrid::FGridCoordinate> PlainArea;
	std::vector<FGrid::FGridCoordinate> TaggedArea;
	Plain.GetWeightedNeighbours({5, 5}, PlainArea, 3, true);
	Tagged.GetWeightedNeighbours({5, 5}, TaggedArea, 3, true);
	TestEqual(TEXT("Reachable area size unchanged"), static_cast<int32>(TaggedArea.size()),
	          static_cast<int32>(PlainArea.size()));

	return true;
}

GRID_TEST(TileTagDoesNotAffectBlockingOrSightTest)

bool TileTagDoesNotAffectBlockingOrSightTest::RunTest(const FString& Parameters)
{
	FGrid Grid = TestsHelper::CreateTestGrid();

	Grid.SetTileTag(2, 0, 5);
	Grid.SetTileTag(3, 0, 5);

	TestFalse(TEXT("Tagged tile is not blocked"), Grid.IsBlocked(2, 0));
	TestTrue(TEXT("Line of sight across tagged tiles (blocked check)"), Grid.HasLineOfSight({0, 0}, {5, 0}, true));
	TestTrue(TEXT("Line of sight across tagged tiles"), Grid.HasLineOfSight({0, 0}, {5, 0}, false));

	// Tagging does not unblock an impassable tile either.
	Grid.SetTile(4, 0, FGrid::kImpassableWeight, 5);
	TestTrue(TEXT("Impassable tile stays blocked"), Grid.IsBlocked(4, 0));
	TestFalse(TEXT("Impassable tile still blocks sight"), Grid.HasLineOfSight({0, 0}, {5, 0}, false));

	return true;
}
