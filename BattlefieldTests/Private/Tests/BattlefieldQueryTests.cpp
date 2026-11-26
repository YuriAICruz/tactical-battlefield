#include "Framework/AutomationShim.h"
#include "Core/BaseUnit.h"
#include "Core/BattleController.h"
#include "Tests/TestsHelper.h"


[[maybe_unused]] static void CreateTestEnemies(FBattleController& controller, FGrid::FGridCoordinate center,
                       std::vector<const FBaseUnit*>& enemies)
{
	enemies.push_back(controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                       center + FGrid::FGridCoordinate{1, 0}));
	enemies.push_back(controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                       center + FGrid::FGridCoordinate{-1, 0}));
	enemies.push_back(controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                       center + FGrid::FGridCoordinate{0, 1}));
	enemies.push_back(controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                       TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                       center + FGrid::FGridCoordinate{0, -1}));
}


#define BATTLEFIELD_QUERY_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Query." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )

BATTLEFIELD_QUERY_TEST(GetMovementAreaContainsOriginTest)

bool GetMovementAreaContainsOriginTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	FGrid::FGridCoordinate position{3, 3};

	const FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);

	std::vector<FGrid::FGridCoordinate> area;
	query->GetMovementArea(unit, area);

	TestTrue(TEXT("Area contains current tile"), TestsHelper::ContainsTile(area, position));

	return true;
}

BATTLEFIELD_QUERY_TEST(NoWinnerAtBattleStartTest)

bool NoWinnerAtBattleStartTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                     TestsHelper::kCombatWeapon, {1, 1});

	controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                     TestsHelper::kCombatWeapon, {2, 1});

	TestFalse(TEXT("No winner yet"), query->HasWinner());

	return true;
}

BATTLEFIELD_QUERY_TEST(WinnerQueryReturnsPlayerTest)

bool WinnerQueryReturnsPlayerTest::RunTest(const FString& Parameters)
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
	TestEqual(TEXT("Winner is player"), query->GetWinner(), ETeams::Player);

	return true;
}

BATTLEFIELD_QUERY_TEST(MovementAreaContainsCurrentTileTest)

bool MovementAreaContainsCurrentTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	std::vector<FGrid::FGridCoordinate> area;
	query->GetMovementArea(unit, area);

	TestTrue(TEXT("Contains current tile"), TestsHelper::ContainsTile(area, {3, 3}));

	return true;
}

BATTLEFIELD_QUERY_TEST(MovementAreaRespectsMovementBudgetTest)

bool MovementAreaRespectsMovementBudgetTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	std::vector<FGrid::FGridCoordinate> area;
	query->GetMovementArea(unit, area);

	TestFalse(
		TEXT("Tile outside movement budget"),
		TestsHelper::ContainsTile(area, {8, 8}));

	return true;
}

BATTLEFIELD_QUERY_TEST(MovementAreaExcludesOccupiedTilesTest)

bool MovementAreaExcludesOccupiedTilesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	FGrid::FGridCoordinate blockerPosition{3, 4};
	const FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                              TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                              blockerPosition);
	std::vector<FGrid::FGridCoordinate> area;
	query->GetMovementArea(unit, area);

	TestFalse(TEXT("Blocked tile excluded"), TestsHelper::ContainsTile(area, blockerPosition));

	return true;
}

BATTLEFIELD_QUERY_TEST(AttackableTargetsContainsEnemyInRangeTest)

bool AttackableTargetsContainsEnemyInRangeTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                               TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                               position);
	const FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                              TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                              {4, 3});
	std::vector<const FGridEntity*> targets;
	query->GetAttackableTargets(player, targets);

	TestTrue(TEXT("Enemy found"), std::find(targets.begin(), targets.end(), enemy) != targets.end());

	return true;
}

BATTLEFIELD_QUERY_TEST(AttackableTargetsExcludesFriendliesTest)

bool AttackableTargetsExcludesFriendliesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                               TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                               position);
	const FBaseUnit* ally = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                             TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                             position + FGrid::FGridCoordinate{1, 2});
	std::vector<const FGridEntity*> targets;
	query->GetAttackableTargets(player, targets);

	TestFalse(TEXT("Friendly excluded"), std::find(targets.begin(), targets.end(), ally) != targets.end());

	return true;
}

BATTLEFIELD_QUERY_TEST(AttackableTargetsExcludesOutOfRangeEnemiesTest)

bool AttackableTargetsExcludesOutOfRangeEnemiesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                               TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                               position);
	const FBaseUnit* enemy = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                              TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                              position + FGrid::FGridCoordinate{10, 10});
	std::vector<const FGridEntity*> targets;
	query->GetAttackableTargets(player, targets);

	TestFalse(TEXT("Far enemy excluded"), std::find(targets.begin(), targets.end(), enemy) != targets.end());

	return true;
}

BATTLEFIELD_QUERY_TEST(VisibleEnemiesContainsVisibleEnemyTest)

bool VisibleEnemiesContainsVisibleEnemyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                               TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                               position);
	const FBaseUnit* enemy = controller.SpawnUnit(ETeams::Enemy, TestsHelper::kBaseCombatStats,
	                                              TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                              position + FGrid::FGridCoordinate{3, 3});
	
	
	TestTrue(TEXT("Player not created"), player != nullptr);
	TestTrue(TEXT("Enemy not created"), enemy != nullptr);
	
	std::vector<const FGridEntity*> enemies;
	query->GetVisibleUnits(player, enemies, Invert(ETeams::Player));

	TestTrue(TEXT("Enemy visible"), std::find(enemies.begin(), enemies.end(), enemy) != enemies.end());

	return true;
}

BATTLEFIELD_QUERY_TEST(VisibleEnemiesExcludesBlockedEnemyTest)

bool VisibleEnemiesExcludesBlockedEnemyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	const FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                               TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                               position);
	const FBaseUnit* enemy = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                              TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                              {1, 3});
	
	grid.SetTileWeight(2, 3, FGrid::kImpassableWeight);
	std::vector<const FGridEntity*> enemies;
	query->GetVisibleUnits(player, enemies, Invert(ETeams::Player));

	TestFalse(TEXT("Enemy hidden"), std::find(enemies.begin(), enemies.end(), enemy) != enemies.end());

	return true;
}

BATTLEFIELD_QUERY_TEST(VisibleEnemiesIgnoresDeadEnemiesTest)

bool VisibleEnemiesIgnoresDeadEnemiesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	constexpr FBaseUnit::FCombatWeapon hitKillWeapon{TestsHelper::kBaseCombatStats.maxHealth, 1, 5};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, hitKillWeapon,
	                                         position);
	FBaseUnit* enemy = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                        TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon,
	                                        position + FGrid::FGridCoordinate{1, 0});
	controller.AttackUnitWithWeapon(player, enemy);

	std::vector<const FGridEntity*> enemies;
	query->GetVisibleUnits(player, enemies, Invert(ETeams::Player));

	TestFalse(TEXT("Dead enemy excluded"), std::find(enemies.begin(), enemies.end(), enemy) != enemies.end());

	return true;
}

BATTLEFIELD_QUERY_TEST(TilesInRangeIncludesMaxRangeTileTest)

bool TilesInRangeIncludesMaxRangeTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate origin{5, 5};

	std::vector<FGrid::FGridCoordinate> coords;
	query->GetNeighbours(origin, 5, coords);

	TestTrue(TEXT("Edge tile included"), TestsHelper::ContainsTile(coords, {5, 0}));
	return true;
}

BATTLEFIELD_QUERY_TEST(TilesInRangeExcludesBeyondRangeTileTest)

bool TilesInRangeExcludesBeyondRangeTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate origin{5, 5};

	std::vector<FGrid::FGridCoordinate> coords;
	query->GetNeighbours(origin, 5, coords);

	TestFalse(TEXT("Beyond range excluded"), TestsHelper::ContainsTile(coords, {6, 0}));
	return true;
}

BATTLEFIELD_QUERY_TEST(TilesInRangeIncludesDiagonalTileTest)

bool TilesInRangeIncludesDiagonalTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate origin{4, 4};

	std::vector<FGrid::FGridCoordinate> coords;
	query->GetWeightedNeighbours(origin, 5, coords, true);

	TestTrue(TEXT("Diagonal included"), TestsHelper::ContainsTile(coords, {2, 2}));
	return true;
}

BATTLEFIELD_QUERY_TEST(LineOfSightClearPathTest)

bool LineOfSightClearPathTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{3, 3};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);

	TestTrue(TEXT("Line of Sight exists"), query->HasLineOfSight(player, {5, 1}));

	return true;
}

BATTLEFIELD_QUERY_TEST(LineOfSightBlockedByWallTest)

bool LineOfSightBlockedByWallTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{1, 1};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	grid.SetTileWeight(3, 1, FGrid::kImpassableWeight);
	TestFalse(TEXT("Line of Sight blocked"), query->HasLineOfSight(player, {5, 1}));
	return true;
}

BATTLEFIELD_QUERY_TEST(LineOfSightSameTileTest)

bool LineOfSightSameTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{2, 2};
	FBaseUnit* player = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
	                                         TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	TestTrue(TEXT("Same tile visible"), query->HasLineOfSight(player, {2, 2}));

	return true;
}

BATTLEFIELD_QUERY_TEST(ValidActionsIncludesMoveTest)

bool ValidActionsIncludesMoveTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{2, 2};
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
											 TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	EBattleAction actions = query->GetValidActions(unit);

	TestTrue(TEXT("Move available"), ActionHasFlag(actions, EBattleAction::Move));
	return true;
}

BATTLEFIELD_QUERY_TEST(ValidActionsRemovesAttackWithoutAPTest)

bool ValidActionsRemovesAttackWithoutAPTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{2, 2};
	FBaseUnit::FTurnResources smallTurnResources = {1,1};
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
											 smallTurnResources, TestsHelper::kCombatWeapon, position);
	controller.DoUnitAction(unit);

	EBattleAction actions = query->GetValidActions(unit);

	TestFalse(TEXT("Attack unavailable"), ActionHasFlag(actions, EBattleAction::Attack));
	return true;
}


BATTLEFIELD_QUERY_TEST(ValidActionsRemovesActionWithoutAPTest)

bool ValidActionsRemovesActionWithoutAPTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{2, 2};
	FBaseUnit::FTurnResources smallTurnResources = {1,1};
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
											 smallTurnResources, TestsHelper::kCombatWeapon, position);

	EBattleAction actions = query->GetValidActions(unit, 10);

	TestFalse(TEXT("Not enought AP for action"), ActionHasFlag(actions, EBattleAction::DoActions));
	return true;
}

BATTLEFIELD_QUERY_TEST(DeadUnitHasNoValidActionsTest)

bool DeadUnitHasNoValidActionsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FGrid::FGridCoordinate position{2, 2};
	FBaseUnit* unit = controller.SpawnUnit(ETeams::Player, TestsHelper::kBaseCombatStats,
											 TestsHelper::kBaseTurnResources, TestsHelper::kCombatWeapon, position);
	unit->ReceiveDamage(unit->GetHealth());

	EBattleAction actions = query->GetValidActions(unit);

	TestEqual(TEXT("No actions"), actions, EBattleAction::None);
	return true;
}

BATTLEFIELD_QUERY_TEST(MovementAreaPassThroughAlliesTest)

bool MovementAreaPassThroughAlliesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	const FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {3, 3});
	TestsHelper::SpawnUnit(controller, ETeams::Player, {4, 3});
	TestsHelper::SpawnUnit(controller, ETeams::Enemy, {3, 5});

	std::vector<FGrid::FGridCoordinate> blocked;
	query->GetMovementArea(unit, blocked);
	TestFalse(TEXT("Default: tile behind the ally unreachable"), TestsHelper::ContainsTile(blocked, {5, 3}));
	TestFalse(TEXT("Default: ally tile excluded"), TestsHelper::ContainsTile(blocked, {4, 3}));

	std::vector<FGrid::FGridCoordinate> passing;
	query->GetMovementArea(unit, passing, true);
	TestTrue(TEXT("Pass-through: tile behind the ally reachable"), TestsHelper::ContainsTile(passing, {5, 3}));
	TestFalse(TEXT("Pass-through: cannot end on the ally"), TestsHelper::ContainsTile(passing, {4, 3}));
	TestFalse(TEXT("Pass-through: enemy tile excluded"), TestsHelper::ContainsTile(passing, {3, 5}));
	TestTrue(TEXT("Pass-through: still contains the origin"), TestsHelper::ContainsTile(passing, {3, 3}));

	return true;
}

BATTLEFIELD_QUERY_TEST(MovementCostThroughAllyTest)

bool MovementCostThroughAllyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {3, 3});
	TestsHelper::SpawnUnit(controller, ETeams::Player, {4, 3});

	TestEqual(TEXT("Default: blocked by the ally"), unit->GetMovementCost(&grid, {5, 3}), -1);
	TestEqual(TEXT("Pass-through: two tiles"), unit->GetMovementCost(&grid, {5, 3}, true), 2);
	TestEqual(TEXT("Ally destination is -1"), unit->GetMovementCost(&grid, {4, 3}), -1);
	TestEqual(TEXT("Ally destination is -1 with pass-through"), unit->GetMovementCost(&grid, {4, 3}, true), -1);

	return true;
}

static bool ContainsUnit(const std::vector<const FGridEntity*>& units, const FGridEntity* unit)
{
	return std::ranges::find(units, unit) != units.end();
}

BATTLEFIELD_QUERY_TEST(AttackAreaRangeOneTest)

bool AttackAreaRangeOneTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	unit->SetWeapons({TestsHelper::MakeWeapon("sword", 1, 1)});

	std::vector<FGrid::FGridCoordinate> area;
	controller.GetQueryController()->GetAttackArea(unit, {5, 5}, area);

	TestEqual(TEXT("Four adjacent tiles"), static_cast<int32>(area.size()), 4);
	TestTrue(TEXT("Right"), TestsHelper::ContainsTile(area, {6, 5}));
	TestTrue(TEXT("Up"), TestsHelper::ContainsTile(area, {5, 4}));
	TestFalse(TEXT("Own tile excluded"), TestsHelper::ContainsTile(area, {5, 5}));
	TestFalse(TEXT("Distance 2 excluded"), TestsHelper::ContainsTile(area, {7, 5}));
	return true;
}

BATTLEFIELD_QUERY_TEST(AttackAreaExactRangeTwoTest)

bool AttackAreaExactRangeTwoTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	unit->SetWeapons({TestsHelper::MakeWeapon("bow", 2, 2)});

	std::vector<FGrid::FGridCoordinate> area;
	controller.GetQueryController()->GetAttackArea(unit, {5, 5}, area);

	TestEqual(TEXT("Eight tiles at distance 2"), static_cast<int32>(area.size()), 8);
	TestFalse(TEXT("Adjacent excluded"), TestsHelper::ContainsTile(area, {6, 5}));
	TestTrue(TEXT("Straight two"), TestsHelper::ContainsTile(area, {7, 5}));
	TestTrue(TEXT("Diagonal"), TestsHelper::ContainsTile(area, {6, 6}));
	return true;
}

BATTLEFIELD_QUERY_TEST(AttackAreaBandOneToTwoTest)

bool AttackAreaBandOneToTwoTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	unit->SetWeapons({TestsHelper::MakeWeapon("javelin", 1, 2)});

	std::vector<FGrid::FGridCoordinate> area;
	controller.GetQueryController()->GetAttackArea(unit, {5, 5}, area);

	TestEqual(TEXT("Twelve tiles"), static_cast<int32>(area.size()), 12);
	TestTrue(TEXT("Adjacent included"), TestsHelper::ContainsTile(area, {6, 5}));
	TestTrue(TEXT("Distance 2 included"), TestsHelper::ContainsTile(area, {7, 5}));
	return true;
}

BATTLEFIELD_QUERY_TEST(AttackAreaClippedByGridEdgeTest)

bool AttackAreaClippedByGridEdgeTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {0, 0});
	unit->SetWeapons({TestsHelper::MakeWeapon("sword", 1, 1)});

	std::vector<FGrid::FGridCoordinate> area;
	controller.GetQueryController()->GetAttackArea(unit, {0, 0}, area);

	TestEqual(TEXT("Only two tiles inside the grid"), static_cast<int32>(area.size()), 2);
	return true;
}

BATTLEFIELD_QUERY_TEST(AttackAreaUnarmedIsEmptyTest)

bool AttackAreaUnarmedIsEmptyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	unit->SetWeapons({});

	std::vector<FGrid::FGridCoordinate> area{{1, 1}};
	controller.GetQueryController()->GetAttackArea(unit, {5, 5}, area);
	TestTrue(TEXT("Unarmed: empty area"), area.empty());

	area.push_back({1, 1});
	controller.GetQueryController()->GetAttackArea(nullptr, {5, 5}, area);
	TestTrue(TEXT("Null unit: empty area"), area.empty());
	return true;
}

BATTLEFIELD_QUERY_TEST(AttackableTargetsRespectMinRangeTest)

bool AttackableTargetsRespectMinRangeTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* archer = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* adjacent = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	FBaseUnit* far = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {7, 5});
	FBaseUnit* tooFar = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {8, 5});
	archer->SetWeapons({TestsHelper::MakeWeapon("bow", 2, 2)});

	std::vector<const FGridEntity*> targets;
	controller.GetQueryController()->GetAttackableTargets(archer, targets);

	TestEqual(TEXT("Only the unit at distance 2"), static_cast<int32>(targets.size()), 1);
	TestTrue(TEXT("Distance 2 target"), ContainsUnit(targets, far));
	TestFalse(TEXT("Adjacent not attackable"), ContainsUnit(targets, adjacent));
	TestFalse(TEXT("Distance 3 not attackable"), ContainsUnit(targets, tooFar));
	return true;
}

BATTLEFIELD_QUERY_TEST(SwitchingWeaponChangesTargetsTest)

bool SwitchingWeaponChangesTargetsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {5, 5});
	FBaseUnit* near = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {6, 5});
	FBaseUnit* far = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {7, 5});
	unit->SetWeapons({TestsHelper::MakeWeapon("sword", 1, 1), TestsHelper::MakeWeapon("bow", 2, 2)});

	std::vector<const FGridEntity*> targets;
	controller.GetQueryController()->GetAttackableTargets(unit, targets);
	TestEqual(TEXT("Sword: one target"), static_cast<int32>(targets.size()), 1);
	TestTrue(TEXT("Sword hits the adjacent one"), ContainsUnit(targets, near));

	unit->EquipWeapon(1);
	controller.GetQueryController()->GetAttackableTargets(unit, targets);
	TestEqual(TEXT("Bow: one target"), static_cast<int32>(targets.size()), 1);
	TestTrue(TEXT("Bow hits the far one"), ContainsUnit(targets, far));
	return true;
}

BATTLEFIELD_QUERY_TEST(AttackableTargetsFromHypotheticalTileTest)

bool AttackableTargetsFromHypotheticalTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {8, 8});
	unit->SetWeapons({TestsHelper::MakeWeapon("sword", 1, 1)});

	std::vector<const FGridEntity*> targets;
	controller.GetQueryController()->GetAttackableTargetsFrom(unit, {7, 8}, targets);
	TestEqual(TEXT("From (7,8) the enemy is adjacent"), static_cast<int32>(targets.size()), 1);
	TestTrue(TEXT("It is the enemy"), ContainsUnit(targets, enemy));

	controller.GetQueryController()->GetAttackableTargetsFrom(unit, {1, 2}, targets);
	TestTrue(TEXT("From (1,2) nothing in reach"), targets.empty());

	controller.GetQueryController()->GetAttackableTargets(unit, targets);
	TestTrue(TEXT("From the real position nothing in reach"), targets.empty());
	TestEqual(TEXT("Unit did not move (x)"), grid.GetUnitPosition(unit).x, 1);
	TestEqual(TEXT("Unit did not move (y)"), grid.GetUnitPosition(unit).y, 1);
	return true;
}

BATTLEFIELD_QUERY_TEST(LegacyWeaponTargetsUnchangedTest)

bool LegacyWeaponTargetsUnchangedTest::RunTest(const FString& Parameters)
{
	// Legacy FCombatWeapon (range 5) behaves as before: everything within distance 1..5 is a target.
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {2, 2});
	FBaseUnit* adjacent = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {3, 2});
	FBaseUnit* edge = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {2, 7});
	FBaseUnit* outside = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {9, 9});

	std::vector<const FGridEntity*> targets;
	controller.GetQueryController()->GetAttackableTargets(unit, targets);
	TestTrue(TEXT("Adjacent target"), ContainsUnit(targets, adjacent));
	TestTrue(TEXT("Target at distance 5"), ContainsUnit(targets, edge));
	TestFalse(TEXT("Target at distance 14"), ContainsUnit(targets, outside));
	return true;
}
