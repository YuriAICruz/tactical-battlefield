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

BATTLEFIELD_CONTROLLER_TEST(SpawnCopiesNameAndAttributesTest)

bool SpawnCopiesNameAndAttributesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1},
	                                       "Alm", {{"STR", 12}, {"level", 3}});

	TestNotNull(TEXT("unit spawned"), unit);
	TestEqual(TEXT("Name"), unit->GetName(), FName("Alm"));
	TestEqual(TEXT("STR"), unit->GetAttribute("STR"), 12);
	TestEqual(TEXT("level"), unit->GetAttribute("level"), 3);
	TestEqual(TEXT("Attribute count"), static_cast<int32>(unit->GetAttributes().size()), 2);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(SpawnWithoutAttributesKeepsDefaultsTest)

bool SpawnWithoutAttributesKeepsDefaultsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, {1, 1});

	TestNotNull(TEXT("unit spawned"), unit);
	TestTrue(TEXT("Name empty"), unit->GetName().empty());
	TestTrue(TEXT("No attributes"), unit->GetAttributes().empty());
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(StartBattleCopiesUnitDataAttributesTest)

bool StartBattleCopiesUnitDataAttributesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	std::vector<FBaseUnit*> spawned;
	controller.OnUnitSpawned = [&](FBaseUnit* unit, const FGrid::FGridCoordinate&) { spawned.push_back(unit); };

	FBattleController::FBattleStartData data;
	FBattleController::FUnitData hero{
		"Hero", ETeams::Player, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon, {1, 1}
	};
	hero.attributes = {{"STR", 8}, {"classId", 2}};
	data.units.push_back(hero);
	data.units.push_back({
		"Grunt", ETeams::Enemy, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon, {5, 5}
	});

	TestTrue(TEXT("Battle started"), controller.StartBattle(data));
	TestEqual(TEXT("Two units"), static_cast<int32>(spawned.size()), 2);
	if (spawned.size() != 2) return true;

	TestEqual(TEXT("Hero name"), spawned[0]->GetName(), FName("Hero"));
	TestEqual(TEXT("Hero STR"), spawned[0]->GetAttribute("STR"), 8);
	TestEqual(TEXT("Hero classId"), spawned[0]->GetAttribute("classId"), 2);
	TestEqual(TEXT("Grunt name"), spawned[1]->GetName(), FName("Grunt"));
	TestTrue(TEXT("Grunt has no attributes"), spawned[1]->GetAttributes().empty());
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(AttackRespectsMinRangeTest)

bool AttackRespectsMinRangeTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* archer = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* adjacent = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	FBaseUnit* far = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {7, 5});
	archer->SetWeapons({TestsHelper::MakeWeapon("bow", 2, 2, 3, 2)});

	const int32 ap = archer->GetActionPoints();
	TestFalse(TEXT("Cannot hit the adjacent enemy"), controller.AttackUnitWithWeapon(archer, adjacent));
	TestEqual(TEXT("Failed attack spends no AP"), archer->GetActionPoints(), ap);
	TestEqual(TEXT("Adjacent enemy untouched"), adjacent->GetHealth(), adjacent->GetMaxHealth());

	TestTrue(TEXT("Can hit the enemy at distance 2"), controller.AttackUnitWithWeapon(archer, far));
	TestEqual(TEXT("Weapon AP cost is spent"), archer->GetActionPoints(), ap - 2);
	TestTrue(TEXT("Damage dealt"), far->GetHealth() < far->GetMaxHealth());
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(AttackUsesEquippedWeaponDamageTest)

bool AttackUsesEquippedWeaponDamageTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	unit->SetWeapons({TestsHelper::MakeWeapon("dagger", 1, 1, 1), TestsHelper::MakeWeapon("axe", 1, 1, 4)});
	unit->EquipWeapon(1);

	TestTrue(TEXT("Attack with the axe"), controller.AttackUnitWithWeapon(unit, enemy));
	TestEqual(TEXT("Axe damage"), enemy->GetMaxHealth() - enemy->GetHealth(), 4);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(UnarmedControllerAttackFailsTest)

bool UnarmedControllerAttackFailsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	unit->SetWeapons({});

	TestFalse(TEXT("No weapon, no attack"), controller.AttackUnitWithWeapon(unit, enemy));
	TestEqual(TEXT("Enemy untouched"), enemy->GetHealth(), enemy->GetMaxHealth());
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(StartBattleCopiesUnitDataWeaponsTest)

bool StartBattleCopiesUnitDataWeaponsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	std::vector<FBaseUnit*> spawned;
	controller.OnUnitSpawned = [&](FBaseUnit* unit, const FGrid::FGridCoordinate&) { spawned.push_back(unit); };

	FBattleController::FBattleStartData data;
	FBattleController::FUnitData hero{
		"Hero", ETeams::Player, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon, {1, 1}
	};
	hero.weapons = {TestsHelper::MakeWeapon("sword", 1, 1), TestsHelper::MakeWeapon("bow", 2, 2)};
	data.units.push_back(hero);
	data.units.push_back({
		"Grunt", ETeams::Enemy, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon, {5, 5}
	});

	TestTrue(TEXT("Battle started"), controller.StartBattle(data));
	TestEqual(TEXT("Two units"), static_cast<int32>(spawned.size()), 2);
	if (spawned.size() != 2) return true;

	TestEqual(TEXT("Hero has the two listed weapons"), static_cast<int32>(spawned[0]->GetWeapons().size()), 2);
	TestEqual(TEXT("First weapon equipped"), spawned[0]->GetEquippedWeapon()->id, FName("sword"));
	TestEqual(TEXT("Grunt keeps the legacy weapon only"), static_cast<int32>(spawned[1]->GetWeapons().size()), 1);
	TestEqual(TEXT("Grunt legacy range"), spawned[1]->GetWeaponAttackRange(), TestsHelper::kCombatWeapon.range);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookUnsetUsesDefaultFormulaTest)

bool ResolveHookUnsetUsesDefaultFormulaTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	const int32 expected = attacker->CalculateWeaponAttackDamage({5, 5}, {6, 5});
	FBattleAttackEvent last(nullptr, nullptr, -1);
	controller.OnUnitDidAttacked = [&](const FBattleAttackEvent& e) { last = e; };

	TestTrue(TEXT("Attack succeeds"), controller.AttackUnitWithWeapon(attacker, target));
	TestEqual(TEXT("Default damage applied"), target->GetMaxHealth() - target->GetHealth(), expected);
	TestEqual(TEXT("Event damage"), last.Damage, expected);
	TestTrue(TEXT("Default is a hit"), last.Hit);
	TestFalse(TEXT("Default is not a crit"), last.Critical);
	TestEqual(TEXT("Default userValue"), last.UserValue, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookDamageIsAppliedTest)

bool ResolveHookDamageIsAppliedTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	attacker->SetWeapons({TestsHelper::MakeWeapon("sword", 1, 1, 2, 2)});

	const FBaseUnit* seenAttacker = nullptr;
	const FBaseUnit* seenTarget = nullptr;
	FName seenWeapon;
	int32 damagedEvents = 0;
	int32 damageSeen = 0;
	controller.OnResolveAttack = [&](FBaseUnit& a, FBaseUnit& t, const FWeaponDef& w)
	{
		seenAttacker = &a;
		seenTarget = &t;
		seenWeapon = w.id;
		return FAttackResult{7, true, false, 0};
	};
	controller.OnUnitDamaged = [&](const FBattleDamageEvent& e)
	{
		++damagedEvents;
		damageSeen = e.Damage;
	};

	const int32 ap = attacker->GetActionPoints();
	TestTrue(TEXT("Attack succeeds"), controller.AttackUnitWithWeapon(attacker, target));
	TestEqual(TEXT("Hook got the attacker"), seenAttacker, static_cast<const FBaseUnit*>(attacker));
	TestEqual(TEXT("Hook got the target"), seenTarget, static_cast<const FBaseUnit*>(target));
	TestEqual(TEXT("Hook got the equipped weapon"), seenWeapon, FName("sword"));
	TestEqual(TEXT("Hook damage applied"), target->GetHealth(), target->GetMaxHealth() - 7);
	TestEqual(TEXT("OnUnitDamaged fired once"), damagedEvents, 1);
	TestEqual(TEXT("OnUnitDamaged damage"), damageSeen, 7);
	TestEqual(TEXT("AP spent"), attacker->GetActionPoints(), ap - 2);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookMissSpendsApWithoutDamageTest)

bool ResolveHookMissSpendsApWithoutDamageTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	int32 attackEvents = 0;
	int32 damagedEvents = 0;
	FBattleAttackEvent last(nullptr, nullptr, -1);
	controller.OnResolveAttack = [](FBaseUnit&, FBaseUnit&, const FWeaponDef&)
	{
		return FAttackResult{9, false, false, 3}; // damage is ignored on a miss
	};
	controller.OnUnitDidAttacked = [&](const FBattleAttackEvent& e)
	{
		++attackEvents;
		last = e;
	};
	controller.OnUnitDamaged = [&](const FBattleDamageEvent&) { ++damagedEvents; };

	const int32 ap = attacker->GetActionPoints();
	TestTrue(TEXT("A miss is still a successful action"), controller.AttackUnitWithWeapon(attacker, target));
	TestEqual(TEXT("Target untouched"), target->GetHealth(), target->GetMaxHealth());
	TestEqual(TEXT("AP spent"), attacker->GetActionPoints(), ap - attacker->GetWeaponAttackCost());
	TestEqual(TEXT("Attack event fired"), attackEvents, 1);
	TestEqual(TEXT("Event damage is 0"), last.Damage, 0);
	TestFalse(TEXT("Event says miss"), last.Hit);
	TestEqual(TEXT("userValue forwarded on a miss"), last.UserValue, 3);
	TestEqual(TEXT("OnUnitDamaged did not fire"), damagedEvents, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookEventPayloadTest)

bool ResolveHookEventPayloadTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	FBattleAttackEvent last(nullptr, nullptr, -1);
	controller.OnResolveAttack = [](FBaseUnit&, FBaseUnit&, const FWeaponDef&)
	{
		return FAttackResult{3, true, true, 42};
	};
	controller.OnUnitDidAttacked = [&](const FBattleAttackEvent& e) { last = e; };

	TestTrue(TEXT("Attack succeeds"), controller.AttackUnitWithWeapon(attacker, target));
	TestEqual(TEXT("Attacker"), last.Attacker, static_cast<const FBaseUnit*>(attacker));
	TestEqual(TEXT("Target"), last.Target, static_cast<const FBaseUnit*>(target));
	TestEqual(TEXT("Damage"), last.Damage, 3);
	TestTrue(TEXT("Hit"), last.Hit);
	TestTrue(TEXT("Critical"), last.Critical);
	TestEqual(TEXT("userValue"), last.UserValue, 42);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookKillTriggersDeathTest)

bool ResolveHookKillTriggersDeathTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	int32 deaths = 0;
	controller.OnUnitDied = [&](const FBaseUnit* u) { if (u == target) ++deaths; };
	controller.OnResolveAttack = [](FBaseUnit&, FBaseUnit&, const FWeaponDef&)
	{
		return FAttackResult{999, true, true, 0};
	};

	TestTrue(TEXT("Attack succeeds"), controller.AttackUnitWithWeapon(attacker, target));
	TestFalse(TEXT("Target is dead"), target->IsAlive());
	TestEqual(TEXT("Death event fired once"), deaths, 1);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookNegativeDamageIsClampedTest)

bool ResolveHookNegativeDamageIsClampedTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	FBattleAttackEvent last(nullptr, nullptr, -1);
	controller.OnResolveAttack = [](FBaseUnit&, FBaseUnit&, const FWeaponDef&)
	{
		return FAttackResult{-5, true, false, 0};
	};
	controller.OnUnitDidAttacked = [&](const FBattleAttackEvent& e) { last = e; };

	TestTrue(TEXT("Attack succeeds"), controller.AttackUnitWithWeapon(attacker, target));
	TestEqual(TEXT("No healing through a negative hit"), target->GetHealth(), target->GetMaxHealth());
	TestEqual(TEXT("Event damage clamped to 0"), last.Damage, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookNotCalledWhenAttackInvalidTest)

bool ResolveHookNotCalledWhenAttackInvalidTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* farTarget = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {9, 9});
	FBaseUnit* ally = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 2});

	int32 calls = 0;
	controller.OnResolveAttack = [&](FBaseUnit&, FBaseUnit&, const FWeaponDef&)
	{
		++calls;
		return FAttackResult{1, true, false, 0};
	};

	TestFalse(TEXT("Out of range"), controller.AttackUnitWithWeapon(attacker, farTarget));
	TestFalse(TEXT("Friendly fire"), controller.AttackUnitWithWeapon(attacker, ally));
	TestEqual(TEXT("Hook never called"), calls, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PreviewAttackAppliesNothingTest)

bool PreviewAttackAppliesNothingTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	int32 events = 0;
	controller.OnUnitDidAttacked = [&](const FBattleAttackEvent&) { ++events; };
	controller.OnUnitDamaged = [&](const FBattleDamageEvent&) { ++events; };
	controller.OnResolveAttack = [](FBaseUnit&, FBaseUnit&, const FWeaponDef&)
	{
		return FAttackResult{6, true, true, 11};
	};

	const int32 ap = attacker->GetActionPoints();
	const FAttackResult preview = controller.PreviewAttack(attacker, target);
	TestEqual(TEXT("Preview damage"), preview.damage, 6);
	TestTrue(TEXT("Preview hit"), preview.hit);
	TestTrue(TEXT("Preview critical"), preview.critical);
	TestEqual(TEXT("Preview userValue"), preview.userValue, 11);
	TestEqual(TEXT("Target untouched"), target->GetHealth(), target->GetMaxHealth());
	TestEqual(TEXT("AP untouched"), attacker->GetActionPoints(), ap);
	TestEqual(TEXT("No events fired"), events, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PreviewAttackMatchesRealAttackTest)

bool PreviewAttackMatchesRealAttackTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	const FAttackResult preview = controller.PreviewAttack(attacker, target);
	TestEqual(TEXT("No hook: preview is the default formula"), preview.damage,
	          attacker->CalculateWeaponAttackDamage({5, 5}, {6, 5}));
	TestTrue(TEXT("No hook: preview is a hit"), preview.hit);

	controller.AttackUnitWithWeapon(attacker, target);
	TestEqual(TEXT("Real attack deals the previewed damage"), target->GetMaxHealth() - target->GetHealth(),
	          preview.damage);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PreviewAttackInvalidInputsTest)

bool PreviewAttackInvalidInputsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});

	TestFalse(TEXT("Null attacker"), controller.PreviewAttack(nullptr, target).hit);
	TestFalse(TEXT("Null target"), controller.PreviewAttack(attacker, nullptr).hit);

	attacker->SetWeapons({});
	const FAttackResult unarmed = controller.PreviewAttack(attacker, target);
	TestFalse(TEXT("Unarmed attacker"), unarmed.hit);
	TestEqual(TEXT("Unarmed damage"), unarmed.damage, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(ResolveHookMayReadQueriesTest)

bool ResolveHookMayReadQueriesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* attacker = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* target = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	TestsHelper::SpawnUnit(controller, ETeams::Enemy, {5, 6});

	// Damage = number of enemies the attacker could reach: a read-only query inside the hook is allowed.
	controller.OnResolveAttack = [&](FBaseUnit& a, FBaseUnit&, const FWeaponDef&)
	{
		std::vector<const FGridEntity*> reachable;
		controller.GetQueryController()->GetAttackableTargets(&a, reachable);
		return FAttackResult{static_cast<int32>(reachable.size()), true, false, 0};
	};

	TestTrue(TEXT("Attack succeeds"), controller.AttackUnitWithWeapon(attacker, target));
	TestEqual(TEXT("Damage came from the query"), target->GetMaxHealth() - target->GetHealth(), 2);
	return true;
}

static FActionDef MakeAction(const FName& id, int32 apCost = 1, bool endsTurn = false)
{
	FActionDef action;
	action.id = id;
	action.name = id;
	action.apCost = apCost;
	action.endsUnitTurn = endsTurn;
	action.execute = [](FBaseUnit&, FGrid::FGridCoordinate) { return true; };
	return action;
}

BATTLEFIELD_CONTROLLER_TEST(RegisterAndListActionsTest)

bool RegisterAndListActionsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);

	TestTrue(TEXT("No actions by default"), controller.GetActions().empty());
	TestTrue(TEXT("Register heal"), controller.RegisterAction(MakeAction("heal")));
	TestTrue(TEXT("Register rescue"), controller.RegisterAction(MakeAction("rescue")));
	TestEqual(TEXT("Two actions"), static_cast<int32>(controller.GetActions().size()), 2);
	TestEqual(TEXT("Order kept"), controller.GetActions()[0].id, FName("heal"));

	FActionDef replaced = MakeAction("heal", 3);
	TestTrue(TEXT("Same id replaces"), controller.RegisterAction(replaced));
	TestEqual(TEXT("Still two actions"), static_cast<int32>(controller.GetActions().size()), 2);
	TestEqual(TEXT("Cost replaced"), controller.GetActions()[0].apCost, 3);

	TestFalse(TEXT("Empty id rejected"), controller.RegisterAction(MakeAction("")));
	TestTrue(TEXT("Unregister"), controller.UnregisterAction("heal"));
	TestFalse(TEXT("Unregister unknown"), controller.UnregisterAction("heal"));
	TestEqual(TEXT("One action left"), static_cast<int32>(controller.GetActions().size()), 1);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PerformActionSpendsApAndFiresEventsTest)

bool PerformActionSpendsApAndFiresEventsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});

	FGrid::FGridCoordinate seenTarget{-1, -1};
	FActionDef action = MakeAction("item", 2);
	action.execute = [&](FBaseUnit&, FGrid::FGridCoordinate target)
	{
		seenTarget = target;
		return true;
	};
	controller.RegisterAction(action);

	int32 legacyEvents = 0;
	FName seenId;
	const FBaseUnit* seenUnit = nullptr;
	controller.OnUnitActionPerformed = [&](const FBaseUnit*) { ++legacyEvents; };
	controller.OnActionPerformed = [&](const FBaseUnit* u, const FName& id)
	{
		seenUnit = u;
		seenId = id;
	};

	const int32 ap = unit->GetActionPoints();
	TestTrue(TEXT("Perform"), controller.PerformAction(unit, "item", {4, 5}));
	TestEqual(TEXT("AP spent"), unit->GetActionPoints(), ap - 2);
	TestEqual(TEXT("Target passed to execute (x)"), seenTarget.x, 4);
	TestEqual(TEXT("Target passed to execute (y)"), seenTarget.y, 5);
	TestEqual(TEXT("Legacy event fired"), legacyEvents, 1);
	TestEqual(TEXT("Event carries the id"), seenId, FName("item"));
	TestEqual(TEXT("Event carries the unit"), seenUnit, static_cast<const FBaseUnit*>(unit));
	TestFalse(TEXT("Not ending the turn: unit did not wait"), unit->HasWaited());
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PerformActionExecuteFailureSpendsNothingTest)

bool PerformActionExecuteFailureSpendsNothingTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});

	FActionDef action = MakeAction("fail", 2, true);
	action.execute = [](FBaseUnit&, FGrid::FGridCoordinate) { return false; };
	controller.RegisterAction(action);

	int32 events = 0;
	controller.OnUnitActionPerformed = [&](const FBaseUnit*) { ++events; };
	controller.OnActionPerformed = [&](const FBaseUnit*, const FName&) { ++events; };
	controller.OnUnitWaited = [&](const FBaseUnit*) { ++events; };

	const int32 ap = unit->GetActionPoints();
	TestFalse(TEXT("Perform fails"), controller.PerformAction(unit, "fail", {0, 0}));
	TestEqual(TEXT("AP untouched"), unit->GetActionPoints(), ap);
	TestFalse(TEXT("Unit did not wait"), unit->HasWaited());
	TestEqual(TEXT("No events"), events, 0);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PerformActionEndsUnitTurnTest)

bool PerformActionEndsUnitTurnTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	controller.SetRequireWait(true);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});
	controller.RegisterAction(MakeAction("wait", 0, true));

	int32 waited = 0;
	controller.OnUnitWaited = [&](const FBaseUnit*) { ++waited; };

	TestTrue(TEXT("Perform a zero-cost end-turn action"), controller.PerformAction(unit, "wait"));
	TestTrue(TEXT("Unit waited"), unit->HasWaited());
	TestEqual(TEXT("OnUnitWaited fired"), waited, 1);
	TestFalse(TEXT("With RequireWait the unit cannot move anymore"), controller.MoveUnit(unit, {2, 3}));
	TestFalse(TEXT("Nor perform another action"), controller.PerformAction(unit, "wait"));
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PerformActionRejectsInvalidCallsTest)

bool PerformActionRejectsInvalidCallsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* player = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {7, 7});
	controller.RegisterAction(MakeAction("heal"));
	controller.RegisterAction(MakeAction("expensive", 99));
	FActionDef locked = MakeAction("locked");
	locked.canUse = [](const FBaseUnit&) { return false; };
	controller.RegisterAction(locked);

	TestFalse(TEXT("Wrong team"), controller.PerformAction(enemy, "heal"));
	TestFalse(TEXT("Null unit"), controller.PerformAction(nullptr, "heal"));
	TestFalse(TEXT("Unknown id"), controller.PerformAction(player, "nope"));
	TestFalse(TEXT("Not enough AP"), controller.PerformAction(player, "expensive"));
	TestFalse(TEXT("canUse false"), controller.PerformAction(player, "locked"));
	TestEqual(TEXT("AP untouched by all rejections"), player->GetActionPoints(), player->GetMaxActionPoints());
	TestTrue(TEXT("The valid one still works"), controller.PerformAction(player, "heal"));
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(PerformActionMayMutateBattleTest)

bool PerformActionMayMutateBattleTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {3, 2});

	// An "attack" action defined entirely on the client side, built on the existing controller calls.
	FActionDef attack = MakeAction("attack", 0);
	attack.execute = [&](FBaseUnit& self, FGrid::FGridCoordinate)
	{
		return controller.AttackUnitRaw(&self, enemy, 1, 1, 4);
	};
	controller.RegisterAction(attack);

	TestTrue(TEXT("Action performed"), controller.PerformAction(unit, "attack"));
	TestEqual(TEXT("Enemy took the damage"), enemy->GetHealth(), enemy->GetMaxHealth() - 4);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(DoUnitActionDelegatesToDefaultActionTest)

bool DoUnitActionDelegatesToDefaultActionTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});

	int32 legacyEvents = 0;
	controller.OnUnitActionPerformed = [&](const FBaseUnit*) { ++legacyEvents; };

	// Not registered: the old stub (cost 1, no effect, event) is unchanged.
	const int32 ap = unit->GetActionPoints();
	TestTrue(TEXT("Legacy stub works"), controller.DoUnitAction(unit));
	TestEqual(TEXT("Legacy cost is 1"), unit->GetActionPoints(), ap - 1);
	TestEqual(TEXT("Legacy event"), legacyEvents, 1);

	// Registered: DoUnitAction runs the "default" action instead.
	int32 executed = 0;
	FActionDef def = MakeAction("default", 3);
	def.execute = [&](FBaseUnit&, FGrid::FGridCoordinate) { ++executed; return true; };
	controller.RegisterAction(def);
	TestTrue(TEXT("Delegated"), controller.DoUnitAction(unit));
	TestEqual(TEXT("execute ran"), executed, 1);
	TestEqual(TEXT("Delegated cost is 3"), unit->GetActionPoints(), ap - 1 - 3);
	return true;
}

BATTLEFIELD_CONTROLLER_TEST(GetTargetsCallbackIsReachableTest)

bool GetTargetsCallbackIsReachableTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});

	FActionDef action = MakeAction("heal");
	action.getTargets = [](const FBaseUnit&, std::vector<FGrid::FGridCoordinate>& out)
	{
		out.push_back({1, 1});
		out.push_back({3, 3});
	};
	controller.RegisterAction(action);

	std::vector<FGrid::FGridCoordinate> targets;
	controller.GetActions()[0].getTargets(*unit, targets);
	TestEqual(TEXT("Two tiles"), static_cast<int32>(targets.size()), 2);
	return true;
}
