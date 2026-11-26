#include "Framework/AutomationShim.h"
#include <type_traits>
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

BATTLEFIELD_CONTROLLER_TEST(MoveEventCarriesUnitAndOriginTest)

bool MoveEventCarriesUnitAndOriginTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});

	int events = 0;
	const FBaseUnit* eventUnit = nullptr;
	FGrid::FGridCoordinate eventFrom{-1, -1};
	FGrid::FGridCoordinate eventTo{-1, -1};
	controller.OnUnitMoved = [&](const FBattleMoveEvent& event)
	{
		events++;
		eventUnit = event.Unit;
		eventFrom = event.From;
		eventTo = event.To;
	};

	TestTrue(TEXT("Move succeeds"), controller.MoveUnit(unit, {1, 3}));
	TestEqual(TEXT("One event"), events, 1);
	TestTrue(TEXT("Event unit is the mover"), eventUnit == unit);
	TestEqual(TEXT("From X"), eventFrom.x, 1);
	TestEqual(TEXT("From Y"), eventFrom.y, 1);
	TestEqual(TEXT("To X"), eventTo.x, 1);
	TestEqual(TEXT("To Y"), eventTo.y, 3);

	// MoveUnitRaw reports the current origin, not the spawn tile
	TestTrue(TEXT("Raw move succeeds"), controller.MoveUnitRaw(unit, {2, 3}, 1));
	TestEqual(TEXT("Two events"), events, 2);
	TestEqual(TEXT("Raw From X"), eventFrom.x, 1);
	TestEqual(TEXT("Raw From Y"), eventFrom.y, 3);
	TestEqual(TEXT("Raw To X"), eventTo.x, 2);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(HealAllyTest)

bool HealAllyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* healer = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* ally = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 1});
	ally->ReceiveDamage(5);
	const int32 before = ally->GetHealth();

	bool bHealEvent = false;
	controller.OnUnitDidHealed = [&](const FBattleHealEvent& event)
	{
		bHealEvent = event.Target == ally && event.Attacker == healer;
	};

	TestTrue(TEXT("Heal ally succeeds"), controller.HealUnitRaw(healer, ally, 5, 1, 3));
	TestEqual(TEXT("Ally health"), ally->GetHealth(), before + 3);
	TestTrue(TEXT("Heal event fired"), bHealEvent);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(HealEnemyIsRejectedTest)

bool HealEnemyIsRejectedTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* healer = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {2, 1});
	enemy->ReceiveDamage(5);
	const int32 before = enemy->GetHealth();
	const int32 apBefore = healer->GetActionPoints();

	TestFalse(TEXT("Heal enemy rejected"), controller.HealUnitRaw(healer, enemy, 5, 1, 3));
	TestEqual(TEXT("Enemy health untouched"), enemy->GetHealth(), before);
	TestEqual(TEXT("AP not spent"), healer->GetActionPoints(), apBefore);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(DoUnitActionRejectsOtherTeamTest)

bool DoUnitActionRejectsOtherTeamTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* player = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {5, 5});

	const int32 enemyAp = enemy->GetActionPoints();
	bool bPerformed = false;
	controller.OnUnitActionPerformed = [&](const FBaseUnit*) { bPerformed = true; };

	TestFalse(TEXT("Enemy cannot act on the player turn"), controller.DoUnitAction(enemy));
	TestEqual(TEXT("Enemy AP untouched"), enemy->GetActionPoints(), enemyAp);
	TestFalse(TEXT("No action event"), bPerformed);
	TestFalse(TEXT("Null unit rejected"), controller.DoUnitAction(nullptr));

	TestTrue(TEXT("Current team can act"), controller.DoUnitAction(player));
	TestTrue(TEXT("Action event"), bPerformed);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(MoveCrossesAllyTest)

bool MoveCrossesAllyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {3, 3});
	FBaseUnit* ally = TestsHelper::SpawnUnit(controller, ETeams::Player, {4, 3});

	TestFalse(TEXT("Cannot stop on the ally"), controller.MoveUnit(unit, {4, 3}));
	TestTrue(TEXT("Crosses the ally to the free tile behind"), controller.MoveUnit(unit, {5, 3}));
	TestEqual(TEXT("Unit X"), grid.GetUnitPosition(unit).x, 5);
	TestEqual(TEXT("Ally stayed"), grid.GetUnitPosition(ally).x, 4);
	TestEqual(TEXT("Cost is two tiles"), unit->GetMovementPoints(), TestsHelper::kBaseTurnResources.move - 2);

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(MoveCannotCrossEnemyTest)

bool MoveCannotCrossEnemyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {3, 3});
	TestsHelper::SpawnUnit(controller, ETeams::Enemy, {4, 3});

	TestFalse(TEXT("Cannot cross an enemy"), controller.MoveUnit(unit, {5, 3}));

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(StartBattleClearsStaleOccupancyTest)

bool StartBattleClearsStaleOccupancyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBattleController::FBattleStartData data;
	data.units.push_back({"", ETeams::Player, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                      TestsHelper::kCombatWeapon, {1, 1}});
	data.units.push_back({"", ETeams::Enemy, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                      TestsHelper::kCombatWeapon, {2, 1}});

	TestTrue(TEXT("First battle starts"), controller.StartBattle(data));
	TestTrue(TEXT("Tile occupied"), grid.IsBlocked(1, 1));

	// Same grid, same positions: used to fail because the tiles still pointed at destroyed units
	TestTrue(TEXT("Second battle on the same grid starts"), controller.StartBattle(data));
	TestNotNull(TEXT("New unit on the tile"), controller.GetQueryController()->GetUnitAt({1, 1}));

	FBattleController::FBattleStartData empty;
	TestTrue(TEXT("Empty battle starts"), controller.StartBattle(empty));
	TestFalse(TEXT("First tile freed"), grid.IsBlocked(1, 1));
	TestFalse(TEXT("Second tile freed"), grid.IsBlocked(2, 1));

	return true;
}

BATTLEFIELD_CONTROLLER_TEST(HealEventAliasTest)

bool HealEventAliasTest::RunTest(const FString& Parameters)
{
	// BattleHealEvent is kept as an alias of FBattleHealEvent for one release
	static_assert(std::is_same_v<BattleHealEvent, FBattleHealEvent>);
	FBattleHealEvent event(nullptr, nullptr, 4);
	BattleHealEvent legacy = event;
	TestEqual(TEXT("Value"), legacy.Value, 4);
	return true;
}
