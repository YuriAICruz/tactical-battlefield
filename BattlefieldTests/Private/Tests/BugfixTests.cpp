#include "Framework/AutomationShim.h"
#include <type_traits>
#include "Core/BaseUnit.h"
#include "Core/BattleController.h"
#include "Core/TeamSystem.h"
#include "Core/Troupe.h"
#include "Tests/TestsHelper.h"

// Tests for docs/features/01-bugfixes.md

#define BATTLEFIELD_BUGFIX_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Bugfix." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )

static FBaseUnit* SpawnTestUnit(FBattleController& controller, ETeams team, FGrid::FGridCoordinate position)
{
	return controller.SpawnUnit(team, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                            TestsHelper::kCombatWeapon, position);
}

// Grid with a wall at x = 4 that only leaves (4, 3) open, so the only way across is through that tile.
static FGrid CreateCorridorGrid()
{
	FGrid grid = TestsHelper::CreateTestGrid();
	for (int y = 0; y < 10; ++y)
	{
		if (y != 3)
		{
			grid.SetTileWeight(4, y, FGrid::kImpassableWeight);
		}
	}
	return grid;
}

BATTLEFIELD_BUGFIX_TEST(MoveEventCarriesUnitAndOriginTest)

bool MoveEventCarriesUnitAndOriginTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = SpawnTestUnit(controller, ETeams::Player, {1, 1});

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

BATTLEFIELD_BUGFIX_TEST(HealAllyTest)

bool HealAllyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* healer = SpawnTestUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* ally = SpawnTestUnit(controller, ETeams::Player, {2, 1});
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

BATTLEFIELD_BUGFIX_TEST(HealEnemyIsRejectedTest)

bool HealEnemyIsRejectedTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* healer = SpawnTestUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = SpawnTestUnit(controller, ETeams::Enemy, {2, 1});
	enemy->ReceiveDamage(5);
	const int32 before = enemy->GetHealth();
	const int32 apBefore = healer->GetActionPoints();

	TestFalse(TEXT("Heal enemy rejected"), controller.HealUnitRaw(healer, enemy, 5, 1, 3));
	TestEqual(TEXT("Enemy health untouched"), enemy->GetHealth(), before);
	TestEqual(TEXT("AP not spent"), healer->GetActionPoints(), apBefore);

	return true;
}

BATTLEFIELD_BUGFIX_TEST(DoUnitActionRejectsOtherTeamTest)

bool DoUnitActionRejectsOtherTeamTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* player = SpawnTestUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = SpawnTestUnit(controller, ETeams::Enemy, {5, 5});

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

BATTLEFIELD_BUGFIX_TEST(MoveCrossesAllyTest)

bool MoveCrossesAllyTest::RunTest(const FString& Parameters)
{
	FGrid grid = CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = SpawnTestUnit(controller, ETeams::Player, {3, 3});
	FBaseUnit* ally = SpawnTestUnit(controller, ETeams::Player, {4, 3});

	TestFalse(TEXT("Cannot stop on the ally"), controller.MoveUnit(unit, {4, 3}));
	TestTrue(TEXT("Crosses the ally to the free tile behind"), controller.MoveUnit(unit, {5, 3}));
	TestEqual(TEXT("Unit X"), grid.GetUnitPosition(unit).x, 5);
	TestEqual(TEXT("Ally stayed"), grid.GetUnitPosition(ally).x, 4);
	TestEqual(TEXT("Cost is two tiles"), unit->GetMovementPoints(), TestsHelper::kBaseTurnResources.move - 2);

	return true;
}

BATTLEFIELD_BUGFIX_TEST(MoveCannotCrossEnemyTest)

bool MoveCannotCrossEnemyTest::RunTest(const FString& Parameters)
{
	FGrid grid = CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	FBaseUnit* unit = SpawnTestUnit(controller, ETeams::Player, {3, 3});
	SpawnTestUnit(controller, ETeams::Enemy, {4, 3});

	TestFalse(TEXT("Cannot cross an enemy"), controller.MoveUnit(unit, {5, 3}));

	return true;
}

BATTLEFIELD_BUGFIX_TEST(MovementAreaPassThroughAlliesTest)

bool MovementAreaPassThroughAlliesTest::RunTest(const FString& Parameters)
{
	FGrid grid = CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	const FBaseUnit* unit = SpawnTestUnit(controller, ETeams::Player, {3, 3});
	SpawnTestUnit(controller, ETeams::Player, {4, 3});
	SpawnTestUnit(controller, ETeams::Enemy, {3, 5});

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

BATTLEFIELD_BUGFIX_TEST(MovementCostThroughAllyTest)

bool MovementCostThroughAllyTest::RunTest(const FString& Parameters)
{
	FGrid grid = CreateCorridorGrid();
	FBattleController controller = TestsHelper::CreateController(&grid);
	const FBaseUnit* unit = SpawnTestUnit(controller, ETeams::Player, {3, 3});
	SpawnTestUnit(controller, ETeams::Player, {4, 3});

	TestEqual(TEXT("Default: blocked by the ally"), unit->GetMovementCost(&grid, {5, 3}), -1);
	TestEqual(TEXT("Pass-through: two tiles"), unit->GetMovementCost(&grid, {5, 3}, true), 2);
	TestEqual(TEXT("Ally destination is -1"), unit->GetMovementCost(&grid, {4, 3}), -1);
	TestEqual(TEXT("Ally destination is -1 with pass-through"), unit->GetMovementCost(&grid, {4, 3}, true), -1);

	return true;
}

BATTLEFIELD_BUGFIX_TEST(MutualDestructionHasNoWinnerTest)

bool MutualDestructionHasNoWinnerTest::RunTest(const FString& Parameters)
{
	FTeamSystem teamSystem;
	FTroupe troupe(&teamSystem);
	FBaseUnit* player = troupe.CreateActor<FUnit>(TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                                              TestsHelper::kCombatWeapon, ETeams::Player);
	FBaseUnit* enemy = troupe.CreateActor<FUnit>(TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                                             TestsHelper::kCombatWeapon, ETeams::Enemy);

	player->ReceiveDamage(TestsHelper::kBaseCombatStats.health);
	enemy->ReceiveDamage(TestsHelper::kBaseCombatStats.health);

	TestTrue(TEXT("Battle is over"), teamSystem.HasWinner());
	TestEqual(TEXT("Nobody wins"), teamSystem.GetWinner(), ETeams::Null);

	return true;
}

BATTLEFIELD_BUGFIX_TEST(EnemyVictoryTest)

bool EnemyVictoryTest::RunTest(const FString& Parameters)
{
	FTeamSystem teamSystem;
	FTroupe troupe(&teamSystem);
	FBaseUnit* player = troupe.CreateActor<FUnit>(TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                                              TestsHelper::kCombatWeapon, ETeams::Player);
	troupe.CreateActor<FUnit>(TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                          TestsHelper::kCombatWeapon, ETeams::Enemy);

	player->ReceiveDamage(TestsHelper::kBaseCombatStats.health);

	TestTrue(TEXT("Battle is over"), teamSystem.HasWinner());
	TestEqual(TEXT("Enemy wins"), teamSystem.GetWinner(), ETeams::Enemy);

	return true;
}

BATTLEFIELD_BUGFIX_TEST(StartBattleClearsStaleOccupancyTest)

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

BATTLEFIELD_BUGFIX_TEST(HealEventAliasTest)

bool HealEventAliasTest::RunTest(const FString& Parameters)
{
	// BattleHealEvent is kept as an alias of FBattleHealEvent for one release
	static_assert(std::is_same_v<BattleHealEvent, FBattleHealEvent>);
	FBattleHealEvent event(nullptr, nullptr, 4);
	BattleHealEvent legacy = event;
	TestEqual(TEXT("Value"), legacy.Value, 4);
	return true;
}
