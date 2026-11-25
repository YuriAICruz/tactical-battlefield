#include "Framework/AutomationShim.h"
#include "Core/BaseUnit.h"
#include "Core/BattleController.h"
#include "Tests/TestsHelper.h"

#define BATTLEFIELD_CONTROLLER_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Controller." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )


BATTLEFIELD_CONTROLLER_TEST(SpawnPlayerUnitTest)

bool SpawnPlayerUnitTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {2, 3});

	TestNotNull(TEXT("unit spawned"), unit);
	TestEqual(TEXT("X"), grid.GetUnitPosition(unit).x, 2);
	TestEqual(TEXT("Y"), grid.GetUnitPosition(unit).y, 3);
	TestEqual(TEXT("Player team"), unit->GetTeam(), ETeams::Player);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotSpawnOnOccupiedTileTest)

bool CannotSpawnOnOccupiedTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBaseUnit* a = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                          TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});
	const FBaseUnit* b = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                          TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});

	TestNotNull(TEXT("First spawn succeeds"), a);
	TestNull(TEXT("Second spawn fails"), b);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(MoveUnitTest)

bool MoveUnitTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FGrid::FGridCoordinate position = {1, 1};
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);

	FGrid::FGridCoordinate destination = position + FGrid::FGridCoordinate{TestsHelper::kBaseTurnResources.move, 0};
	bool bSuccess = controller.MoveUnit(unit, destination);

	TestTrue(TEXT("Move succeeds"), bSuccess);
	TestEqual(TEXT("X"), grid.GetUnitPosition(unit).x, destination.x);
	TestEqual(TEXT("Y"), grid.GetUnitPosition(unit).y, destination.y);

	return true;
}


BATTLEFIELD_CONTROLLER_TEST(GetUnitMovementAreaTest)

bool GetUnitMovementAreaTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position = {1, 1};
	const FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);

	std::vector<FGrid::FGridCoordinate> movementArea;
	query->GetMovementArea(unit, movementArea);

	TestGreaterThan(TEXT("Movement area should be valid"), static_cast<int>(movementArea.size()), 0);

	if (movementArea.size() == 0)
	{
		return false;
	}

	TestTrue(TEXT("Contains current tile"),
	         TestsHelper::ContainsTile(movementArea, position));
	TestTrue(TEXT("Contains right tile"),
	         TestsHelper::ContainsTile(movementArea, position + FGrid::FGridCoordinate{1, 0}));
	TestTrue(TEXT("Contains left tile"),
	         TestsHelper::ContainsTile(movementArea, position + FGrid::FGridCoordinate{-1, 0}));
	TestTrue(TEXT("Contains final right tile"),
	         TestsHelper::ContainsTile(movementArea,
	                                   position + FGrid::FGridCoordinate{TestsHelper::kBaseTurnResources.move, 0}));
	TestTrue(TEXT("Contains bottom tile"),
	         TestsHelper::ContainsTile(movementArea, position + FGrid::FGridCoordinate{0, -1}));
	TestTrue(TEXT("Contains final top tile"),
	         TestsHelper::ContainsTile(movementArea,
	                                   position + FGrid::FGridCoordinate{0, TestsHelper::kBaseTurnResources.move}));
	TestFalse(TEXT("Does not contain distant tile"),
	          TestsHelper::ContainsTile(movementArea, position + FGrid::FGridCoordinate{
		                                    TestsHelper::kBaseTurnResources.move + 1,
		                                    TestsHelper::kBaseTurnResources.move + 1
	                                    }));

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(MoveConsumesMovementPointsTest)

bool MoveConsumesMovementPointsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FGrid::FGridCoordinate position = {1, 1};
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);

	const int32 Before = unit->GetMovementPoints();
	controller.MoveUnit(unit, position + FGrid::FGridCoordinate{1, 1});
	TestTrue(TEXT("Movement consumed"), unit->GetMovementPoints() < Before);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotMoveFurtherThanMovementAllowanceTest)

bool CannotMoveFurtherThanMovementAllowanceTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {0, 0});

	bool bSuccess = controller.MoveUnit(unit, {
		                                    TestsHelper::kBaseTurnResources.move + 1,
		                                    TestsHelper::kBaseTurnResources.move + 1
	                                    });
	TestFalse(TEXT("Move rejected"), bSuccess);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotMoveEnemyUnitDuringPlayerTurnTest)

bool CannotMoveEnemyUnitDuringPlayerTurnTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});
	bool bSuccess = controller.MoveUnit(enemy, {2, 2});

	TestFalse(TEXT("enemy cannot move"), bSuccess);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotMoveDeadUnitTest)

bool CannotMoveDeadUnitTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	constexpr FBaseUnit::FCombatWeapon hitKillWeapon{TestsHelper::kBaseCombatStats.maxHealth, 1, 5};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, hitKillWeapon, {1, 1});
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, hitKillWeapon, {2, 1});

	controller.EndTurn();
	controller.AttackUnitWithWeapon(enemy, player);
	controller.EndTurn();

	TestTrue(TEXT("Is Dead"), !player->IsAlive());
	bool bMoved = controller.MoveUnit(player, {2, 2});
	TestFalse(TEXT("Dead unit cannot move"), bMoved);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(AttackEnemyTest)

bool AttackEnemyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {2, 1});

	const int32 HealthBefore = enemy->GetHealth();

	bool bSuccess = controller.AttackUnitWithWeapon(player, enemy);

	TestTrue(TEXT("Attack succeeds"), bSuccess);

	TestTrue(TEXT("Enemy damaged"), enemy->GetHealth() < HealthBefore);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(AttackConsumesAPTest)

bool AttackConsumesAPTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {2, 1});

	const int32 APBefore = player->GetActionPoints();

	controller.AttackUnitWithWeapon(player, enemy);

	TestTrue(TEXT("AP spent"), player->GetActionPoints() < APBefore);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotAttackFriendlyUnitTest)

bool CannotAttackFriendlyUnitTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* A = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                    TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});
	FBaseUnit* B = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                    TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {2, 1});

	bool bSuccess = controller.AttackUnitWithWeapon(A, B);

	TestFalse(TEXT("Friendly fire blocked"), bSuccess);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotAttackOutOfRangeTest)

bool CannotAttackOutOfRangeTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {9, 9});

	bool bSuccess = controller.AttackUnitWithWeapon(player, enemy);

	TestFalse(TEXT("Out of range"), bSuccess);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotAttackWithoutAPTest)

bool CannotAttackWithoutAPTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	constexpr FBaseUnit::FTurnResources resources{1, 1};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats, resources,
	                                         TestsHelper::kCombatWeapon, {1, 1});
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {2, 1});

	controller.DoUnitAction(player);

	bool bSuccess = controller.AttackUnitWithWeapon(player, enemy);

	TestFalse(TEXT("No AP"), bSuccess);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(EndTurnAllowsEnemyToActTest)

bool EndTurnAllowsEnemyToActTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});

	controller.EndTurn();

	bool bMoved = controller.MoveUnit(enemy, {2, 1});
	TestTrue(TEXT("enemy can move"), bMoved);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(EndTurnRefreshesResourcesTest)

bool EndTurnRefreshesResourcesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});

	controller.MoveUnit(unit, {2, 1});
	controller.DoUnitAction(unit);

	for (int i = 0; i < static_cast<int>(ETeams::Length); ++i)
	{
		controller.EndTurn();
	}

	TestEqual(TEXT("Movement restored"), unit->GetMovementPoints(), unit->GetMaxMovementPoints());
	TestEqual(TEXT("AP restored"), unit->GetActionPoints(), unit->GetMaxActionPoints());
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PlayerWinsAfterKillingLastEnemyTest)

bool PlayerWinsAfterKillingLastEnemyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	constexpr FBaseUnit::FCombatWeapon hitKillWeapon{TestsHelper::kBaseCombatStats.maxHealth, 1, 5};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, hitKillWeapon, {1, 1});
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, hitKillWeapon, {2, 1});

	controller.AttackUnitWithWeapon(player, enemy);

	TestTrue(TEXT("Winner exists"), query->HasWinner());
	TestEqual(TEXT("Player wins"), query->GetWinner(), ETeams::Player);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(DeadUnitRemovedFromGridTest)

bool DeadUnitRemovedFromGridTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	constexpr FBaseUnit::FCombatWeapon hitKillWeapon{
		TestsHelper::kBaseCombatStats.maxHealth,
		1,
		5
	};

	FBaseUnit* player = controller.SpawnUnit(
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		hitKillWeapon,
		{1, 1});

	FBaseUnit* enemy = controller.SpawnUnit(
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		hitKillWeapon,
		{2, 1});

	controller.AttackUnitWithWeapon(player, enemy);

	TestFalse(TEXT("Enemy should be dead"), enemy->IsAlive());

	const FGrid::FTileData* tile = grid.TryGetTile(2, 1);

	TestNull(TEXT("Tile should be empty after death"), tile->unit);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotAttackDuringOtherTeamsTurnTest)

bool CannotAttackDuringOtherTeamsTurnTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* player = controller.SpawnUnit(
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{1, 1});

	FBaseUnit* enemy = controller.SpawnUnit(
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{2, 1});

	controller.EndTurn(); // Enemy turn

	bool bSuccess = controller.AttackUnitWithWeapon(player, enemy);

	TestFalse(TEXT("Player cannot attack during enemy turn"), bSuccess);

	return true;
}


BATTLEFIELD_CONTROLLER_TEST(EnemyCanAttackOnOwnTurnTest)

bool EnemyCanAttackOnOwnTurnTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* player = controller.SpawnUnit(
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{1, 1});

	FBaseUnit* enemy = controller.SpawnUnit(
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{2, 1});

	controller.EndTurn();

	bool bSuccess = controller.AttackUnitWithWeapon(enemy, player);

	TestTrue(TEXT("Enemy can attack during enemy turn"), bSuccess);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(CannotSpawnOnBlockedTileTest)

bool CannotSpawnOnBlockedTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	grid.SetTileWeight(5,5,FGrid::kImpassableWeight);

	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* unit = controller.SpawnUnit(
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{5, 5});

	TestNull(TEXT("Spawn should fail"), unit);

	return true;
}