#include "Framework/AutomationShim.h"
#include "Core/BaseUnit.h"
#include "Core/BattleController.h"
#include "Events/BattleAttackEvent.h"
#include "Events/BattleDamageEvent.h"
#include "Events/BattleMoveEvent.h"
#include "Tests/TestsHelper.h"

static FBattleController::FBattleStartData CreateSimpleBattle()
{
	FBattleController::FBattleStartData data;

	data.units.push_back({
		"",
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{1, 1}
	});

	data.units.push_back({
		"",
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{2, 1}
	});

	return data;
}

#define BATTLEFIELD_FLOW_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Flow." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )

BATTLEFIELD_FLOW_TEST(StartBattleCreatesUnitsTest)

bool StartBattleCreatesUnitsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	FBattleController::FBattleStartData data = CreateSimpleBattle();
	bool bStarted = controller.StartBattle(data);

	TestTrue(TEXT("Battle started"), bStarted);
	TestEqual(TEXT("Player starts"), query->GetCurrentTeam(), ETeams::Player);
	return true;
}

BATTLEFIELD_FLOW_TEST(CannotStartBattleWithOverlappingUnitsTest)

bool CannotStartBattleWithOverlappingUnitsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	FBattleController::FBattleStartData data;

	data.units.push_back({
		"",
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{3, 3}
	});
	data.units.push_back({
		"",
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		TestsHelper::kCombatWeapon,
		{3, 3}
	});

	bool bStarted = controller.StartBattle(data);

	TestFalse(TEXT("Invalid battle"), bStarted);

	return true;
}


BATTLEFIELD_FLOW_TEST(SingleTurnVictoryTest)

bool SingleTurnVictoryTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FBattleController::FBattleStartData data;
	constexpr FBaseUnit::FCombatWeapon killWeapon{999, 1, 5};

	data.units.push_back({
		"",
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		killWeapon,
		{1, 1}
	});

	data.units.push_back({
		"",
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		killWeapon,
		{2, 1}
	});

	controller.StartBattle(data);

	FBaseUnit* player = query->GetUnitAt({1, 1});
	FBaseUnit* enemy = query->GetUnitAt({2, 1});

	controller.AttackUnitWithWeapon(player, enemy);

	TestTrue(TEXT("Winner exists"), query->HasWinner());
	TestEqual(TEXT("Player wins"), query->GetWinner(), ETeams::Player);

	return true;
}

BATTLEFIELD_FLOW_TEST(FullTurnCycleTest)

bool FullTurnCycleTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	controller.StartBattle(CreateSimpleBattle());
	TestEqual(TEXT("Player"), query->GetCurrentTeam(), ETeams::Player);

	controller.EndTurn();
	TestEqual(TEXT("Enemy"), query->GetCurrentTeam(), ETeams::Enemy);

	controller.EndTurn();
	TestEqual(TEXT("Neutral"), query->GetCurrentTeam(), ETeams::Neutral);

	controller.EndTurn();
	TestEqual(TEXT("Player again"), query->GetCurrentTeam(), ETeams::Player);
	return true;
}

BATTLEFIELD_FLOW_TEST(MultiTurnBattleTest)

bool MultiTurnBattleTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	controller.StartBattle(CreateSimpleBattle());

	FBaseUnit* player = query->GetUnitAt({1, 1});
	FBaseUnit* enemy = query->GetUnitAt({2, 1});

	while (!query->HasWinner())
	{
		if (query->GetCurrentTeam() == ETeams::Player)
		{
			controller.AttackUnitWithWeapon(player, enemy);
		}
		else
		{
			controller.AttackUnitWithWeapon(enemy, player);
		}

		if (!query->HasWinner())
		{
			controller.EndTurn();
		}
	}

	TestTrue(TEXT("Battle completed"), query->HasWinner());

	return true;
}

BATTLEFIELD_FLOW_TEST(AIVsAISimulationTest)

bool AIVsAISimulationTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	FBattleController::FBattleStartData data;

	for (int i = 0; i < 3; ++i)
	{
		data.units.push_back({
			"",
			ETeams::Player,
			TestsHelper::kBaseCombatStats,
			TestsHelper::kBaseTurnResources,
			TestsHelper::kCombatWeapon,
			{1, i + 1}
		});
	}

	for (int i = 0; i < 3; ++i)
	{
		data.units.push_back({
			"",
			ETeams::Enemy,
			TestsHelper::kBaseCombatStats,
			TestsHelper::kBaseTurnResources,
			TestsHelper::kCombatWeapon,
			{8, i + 1}
		});
	}

	controller.StartBattle(data);

	int32 TurnLimit = 100;

	std::vector<FBaseUnit*> units;
	std::vector<FGrid::FGridCoordinate> path;
	static const int32 searchSize = grid.GetHeight() * grid.GetWidth();
	while (!query->HasWinner() && TurnLimit-- > 0)
	{
		query->GetUnitsForCurrentTeam(units);
		for (FBaseUnit* unit : units)
		{
			if (!unit->IsAlive())
			{
				continue;
			}

			FBaseUnit* target = query->FindNearestUnit(unit, searchSize);

			if (!target)
			{
				continue;
			}

			if (!controller.AttackUnitWithWeapon(unit, target))
			{
				const FGrid::FGridCoordinate origin = grid.GetUnitPosition(unit);
				const FGrid::FGridCoordinate destination = grid.GetUnitPosition(target);
				int32 movementSize = grid.GetDistance(origin, destination);
				int32 movetSize = unit->GetMovementPoints();
				query->GetPathTo(unit, target, movementSize, path);

				if (!path.empty())
				{
					if (path.size() > movetSize)
					{
						controller.MoveUnit(unit, path[movetSize]);
					}
					else
					{
						controller.MoveUnit(unit, path.back());
					}
				}
			}
		}

		if (!query->HasWinner())
		{
			controller.EndTurn();
		}
	}

	TestTrue(TEXT("Battle ended"), query->HasWinner());
	TestTrue(TEXT("Turn limit respected"), TurnLimit > 0);

	return true;
}


BATTLEFIELD_FLOW_TEST(DeadUnitNeverReceivesTurnTest)

bool DeadUnitNeverReceivesTurnTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FBattleController::FBattleStartData data;
	constexpr FBaseUnit::FCombatWeapon killWeapon{999, 1, 5};

	data.units.push_back({
		"",
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		killWeapon,
		{1, 1}
	});

	data.units.push_back({
		"",
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		killWeapon,
		{2, 1}
	});

	controller.StartBattle(data);

	FBaseUnit* player = query->GetUnitAt({1, 1});
	FBaseUnit* enemy = query->GetUnitAt({2, 1});

	TestTrue(TEXT("Player found"), player != nullptr);
	TestTrue(TEXT("Enemy found"), enemy != nullptr);

	controller.AttackUnitWithWeapon(player, enemy);

	int32 APBefore = enemy->GetActionPoints();

	for (int i = 0; i < 10; ++i)
	{
		controller.EndTurn();
	}

	TestEqual(TEXT("Dead unit unchanged"), enemy->GetActionPoints(), APBefore);

	return true;
}

// --------------- Events Tests
BATTLEFIELD_FLOW_TEST(BattleStartedEventTest)

bool BattleStartedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	bool bTriggered = false;

	controller.OnBattleStarted =
		[&](const FBattleController::FBattleStartData& Data)
		{
			bTriggered = true;
			TestEqual(TEXT("Unit count"), static_cast<int32>(Data.units.size()), 2);
		};

	controller.StartBattle(CreateSimpleBattle());

	TestTrue(TEXT("Battle started event"), bTriggered);

	return true;
}

BATTLEFIELD_FLOW_TEST(UnitSpawnedEventTest)

bool UnitSpawnedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	int32 SpawnCount = 0;

	controller.OnUnitSpawned = [&](const FBaseUnit*, const FGrid::FGridCoordinate&) { SpawnCount++; };

	controller.StartBattle(CreateSimpleBattle());

	TestEqual(TEXT("Two units spawned"), SpawnCount, 2);

	return true;
}

BATTLEFIELD_FLOW_TEST(TurnEndedEventTest)

bool TurnEndedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	bool bCalled = false;
	ETeams EndedTeam = ETeams::Null;

	controller.OnTurnEnded =
		[&](ETeams Team)
		{
			bCalled = true;
			EndedTeam = Team;
		};

	controller.EndTurn();

	TestTrue(TEXT("Turn ended fired"), bCalled);
	TestEqual(TEXT("Player turn ended"), EndedTeam, ETeams::Player);

	return true;
}

BATTLEFIELD_FLOW_TEST(TurnStartedEventTest)

bool TurnStartedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	bool bCalled = false;
	ETeams StartedTeam = ETeams::Null;

	controller.OnTurnStarted =
		[&](ETeams Team)
		{
			bCalled = true;
			StartedTeam = Team;
		};

	controller.EndTurn();

	TestTrue(TEXT("Turn started fired"), bCalled);
	TestEqual(TEXT("Enemy turn started"), StartedTeam, ETeams::Enemy);

	return true;
}

BATTLEFIELD_FLOW_TEST(UnitMovedEventTest)

bool UnitMovedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	controller.StartBattle(CreateSimpleBattle());

	const FBattleQueries* Query = controller.GetQueryController();

	FBaseUnit* Player = Query->GetUnitAt({1, 1});

	bool bMovedEvent = false;

	controller.OnUnitMoved =
		[&](const FBattleMoveEvent& Event)
		{
			bMovedEvent = true;

			TestEqual(TEXT("Destination X"), Event.To.x, 1);
			TestEqual(TEXT("Destination Y"), Event.To.y, 2);
		};

	controller.MoveUnit(Player, {1, 2});

	TestTrue(TEXT("Move event fired"), bMovedEvent);

	return true;
}

BATTLEFIELD_FLOW_TEST(UnitDamagedEventTest)

bool UnitDamagedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	controller.StartBattle(CreateSimpleBattle());

	const FBattleQueries* Query = controller.GetQueryController();

	FBaseUnit* Player = Query->GetUnitAt({1, 1});
	FBaseUnit* Enemy = Query->GetUnitAt({2, 1});

	bool bDamaged = false;

	controller.OnUnitDamaged =
		[&](const FBattleDamageEvent& Event)
		{
			bDamaged = true;

			TestEqual(TEXT("Target"), Event.Target, static_cast<const FBaseUnit*>(Enemy));
			TestTrue(TEXT("Damage applied"), Event.Damage > 0);
		};

	controller.AttackUnitWithWeapon(Player, Enemy);

	TestTrue(TEXT("Damage event fired"), bDamaged);

	return true;
}

BATTLEFIELD_FLOW_TEST(UnitAttackedEventTest)

bool UnitAttackedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	controller.StartBattle(CreateSimpleBattle());

	const FBattleQueries* Query = controller.GetQueryController();

	FBaseUnit* Player = Query->GetUnitAt({1, 1});
	FBaseUnit* Enemy = Query->GetUnitAt({2, 1});

	bool bAttackEvent = false;

	controller.OnUnitDidAttacked =
		[&](const FBattleAttackEvent& Event)
		{
			bAttackEvent = true;

			TestEqual(TEXT("Attacker"), Event.Attacker, static_cast<const FBaseUnit*>(Player));
			TestEqual(TEXT("Target"), Event.Target, static_cast<const FBaseUnit*>(Enemy));
		};

	controller.AttackUnitWithWeapon(Player, Enemy);

	TestTrue(TEXT("Attack event fired"), bAttackEvent);

	return true;
}

BATTLEFIELD_FLOW_TEST(UnitDiedEventTest)

bool UnitDiedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);

	FBattleController::FBattleStartData data;

	constexpr FBaseUnit::FCombatWeapon KillWeapon{999, 1, 5};

	data.units.push_back({
		"",
		ETeams::Player,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		KillWeapon,
		{1, 1}
	});

	data.units.push_back({
		"",
		ETeams::Enemy,
		TestsHelper::kBaseCombatStats,
		TestsHelper::kBaseTurnResources,
		KillWeapon,
		{2, 1}
	});

	controller.StartBattle(data);

	const FBattleQueries* Query = controller.GetQueryController();

	FBaseUnit* Player = Query->GetUnitAt({1, 1});
	FBaseUnit* Enemy = Query->GetUnitAt({2, 1});

	bool bDied = false;

	controller.OnUnitDied =
		[&](const FBaseUnit* Unit)
		{
			bDied = true;
			TestEqual(TEXT("Dead unit"), Unit, static_cast<const FBaseUnit*>(Enemy));
		};

	controller.AttackUnitWithWeapon(Player, Enemy);

	TestTrue(TEXT("Death event fired"), bDied);

	return true;
}

BATTLEFIELD_FLOW_TEST(WinnerDeclaredEventTest)

bool WinnerDeclaredEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	bool bWinnerDeclared = false;

	controller.OnWinnerDeclared =
		[&](ETeams Winner)
		{
			bWinnerDeclared = true;
			TestEqual(TEXT("Winner"), Winner, ETeams::Player);
		};

	controller.StartBattle(CreateSimpleBattle());

	FBaseUnit* player = query->GetUnitAt({1, 1});
	FBaseUnit* enemy = query->GetUnitAt({2, 1});

	while (!query->HasWinner())
	{
		if (query->GetCurrentTeam() == ETeams::Player)
		{
			controller.AttackUnitWithWeapon(player, enemy);
		}
		else
		{
			controller.AttackUnitWithWeapon(enemy, player);
		}

		if (!query->HasWinner())
		{
			controller.EndTurn();
		}
	}

	TestTrue(TEXT("Winner declared"), bWinnerDeclared);

	return true;
}

BATTLEFIELD_FLOW_TEST(BattleEndedEventTest)

bool BattleEndedEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();

	bool bBattleEnded = false;

	controller.OnBattleEnded =
		[&](ETeams Winner)
		{
			bBattleEnded = true;
			TestEqual(TEXT("Winner"), Winner, ETeams::Player);
		};

	controller.StartBattle(CreateSimpleBattle());

	FBaseUnit* player = query->GetUnitAt({1, 1});
	FBaseUnit* enemy = query->GetUnitAt({2, 1});

	while (!query->HasWinner())
	{
		if (query->GetCurrentTeam() == ETeams::Player)
		{
			controller.AttackUnitWithWeapon(player, enemy);
		}
		else
		{
			controller.AttackUnitWithWeapon(enemy, player);
		}

		if (!query->HasWinner())
		{
			controller.EndTurn();
		}
	}

	TestTrue(TEXT("Battle ended"), bBattleEnded);

	return true;
}


// --------------- Unit turn state (wait) Tests

BATTLEFIELD_FLOW_TEST(UnitCanActFalseAfterWaitRegardlessOfResourcesTest)

bool UnitCanActFalseAfterWaitRegardlessOfResourcesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});

	TestFalse(TEXT("Not waited at spawn"), unit->HasWaited());
	TestTrue(TEXT("Can act at spawn"), unit->CanAct());

	unit->Wait();

	TestTrue(TEXT("Waited"), unit->HasWaited());
	TestTrue(TEXT("Still has AP"), unit->GetActionPoints() > 0);
	TestTrue(TEXT("Still has move"), unit->GetMovementPoints() > 0);
	TestFalse(TEXT("CanAct false after wait"), unit->CanAct());
	return true;
}

BATTLEFIELD_FLOW_TEST(WaitUnitSetsWaitedAndFiresEventTest)

bool WaitUnitSetsWaitedAndFiresEventTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	FBaseUnit* unit = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});

	int32 events = 0;
	const FBaseUnit* eventUnit = nullptr;
	controller.OnUnitWaited = [&](const FBaseUnit* u)
	{
		++events;
		eventUnit = u;
	};

	TestTrue(TEXT("WaitUnit succeeds"), controller.WaitUnit(unit));
	TestTrue(TEXT("Unit waited"), unit->HasWaited());
	TestEqual(TEXT("One event"), events, 1);
	TestTrue(TEXT("Event carries the unit"), eventUnit == unit);
	return true;
}

BATTLEFIELD_FLOW_TEST(WaitUnitRejectsInvalidCasesTest)

bool WaitUnitRejectsInvalidCasesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	FBaseUnit* player = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {2, 1});
	FBaseUnit* victim = TestsHelper::SpawnUnit(controller, ETeams::Player, {3, 1});

	int32 events = 0;
	controller.OnUnitWaited = [&](const FBaseUnit*) { ++events; };

	TestFalse(TEXT("Null unit"), controller.WaitUnit(nullptr));
	TestFalse(TEXT("Not the current team"), controller.WaitUnit(enemy));
	TestFalse(TEXT("Enemy did not wait"), enemy->HasWaited());

	TestTrue(TEXT("First wait"), controller.WaitUnit(player));
	TestFalse(TEXT("Second wait is rejected"), controller.WaitUnit(player));

	victim->ReceiveDamage(1000);
	TestFalse(TEXT("Dead unit"), controller.WaitUnit(victim));
	TestFalse(TEXT("Dead unit did not wait"), victim->HasWaited());

	TestEqual(TEXT("Only the valid wait raised an event"), events, 1);
	return true;
}

BATTLEFIELD_FLOW_TEST(WaitBlocksMoveAndAttackWhenRequiredTest)

bool WaitBlocksMoveAndAttackWhenRequiredTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	controller.SetRequireWait(true);
	FBaseUnit* player = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {2, 1});

	TestTrue(TEXT("Flag is on"), controller.GetRequireWait());
	TestTrue(TEXT("Wait"), controller.WaitUnit(player));

	const int32 ap = player->GetActionPoints();
	const int32 move = player->GetMovementPoints();
	const int32 enemyHealth = enemy->GetHealth();

	TestFalse(TEXT("MoveUnit blocked"), controller.MoveUnit(player, {1, 2}));
	TestFalse(TEXT("MoveUnitRaw blocked"), controller.MoveUnitRaw(player, {1, 2}, 1));
	TestFalse(TEXT("Attack blocked"), controller.AttackUnitWithWeapon(player, enemy));
	TestFalse(TEXT("AttackUnitRaw blocked"), controller.AttackUnitRaw(player, enemy, 5, 1, 1));
	TestFalse(TEXT("DoUnitAction blocked"), controller.DoUnitAction(player));

	TestTrue(TEXT("Unit did not move"), grid.GetUnitPosition(player) == FGrid::FGridCoordinate(1, 1));
	TestEqual(TEXT("AP untouched"), player->GetActionPoints(), ap);
	TestEqual(TEXT("Move untouched"), player->GetMovementPoints(), move);
	TestEqual(TEXT("Enemy undamaged"), enemy->GetHealth(), enemyHealth);
	return true;
}

BATTLEFIELD_FLOW_TEST(WaitedUnitDoesNotBlockOtherUnitsTest)

bool WaitedUnitDoesNotBlockOtherUnitsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	controller.SetRequireWait(true);
	FBaseUnit* waiter = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* other = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 3});

	controller.WaitUnit(waiter);

	TestTrue(TEXT("Other unit still moves"), controller.MoveUnit(other, {1, 4}));
	return true;
}

BATTLEFIELD_FLOW_TEST(WaitClearedOnTeamNextTurnTest)

bool WaitClearedOnTeamNextTurnTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	controller.SetRequireWait(true);
	FBaseUnit* player = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {2, 1});

	controller.WaitUnit(player);
	TestTrue(TEXT("Waited"), player->HasWaited());

	controller.EndTurn(); // Enemy
	TestTrue(TEXT("Still waited during the enemy turn"), player->HasWaited());
	TestFalse(TEXT("Enemy has not waited"), enemy->HasWaited());
	controller.WaitUnit(enemy);
	controller.EndTurn(); // Neutral
	TestTrue(TEXT("Enemy wait persists until its next turn"), enemy->HasWaited());
	TestTrue(TEXT("Player still waited"), player->HasWaited());
	controller.EndTurn(); // Player again

	TestFalse(TEXT("Player wait cleared on its next turn"), player->HasWaited());
	TestTrue(TEXT("Player can act again"), player->CanAct());
	TestTrue(TEXT("Player moves again"), controller.MoveUnit(player, {1, 2}));

	controller.EndTurn(); // Enemy again
	TestFalse(TEXT("Enemy wait cleared on its next turn"), enemy->HasWaited());
	return true;
}

BATTLEFIELD_FLOW_TEST(WaitHasNoEffectOnActionsWhenFlagOffTest)

bool WaitHasNoEffectOnActionsWhenFlagOffTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	FBaseUnit* player = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* enemy = TestsHelper::SpawnUnit(controller, ETeams::Enemy, {2, 1});

	TestFalse(TEXT("Flag defaults to off"), controller.GetRequireWait());
	TestTrue(TEXT("Wait still recorded"), controller.WaitUnit(player));
	TestTrue(TEXT("Unit is marked waited"), player->HasWaited());

	TestTrue(TEXT("Move still works"), controller.MoveUnit(player, {1, 2}));
	TestTrue(TEXT("Attack still works"), controller.AttackUnitWithWeapon(player, enemy));
	return true;
}

BATTLEFIELD_FLOW_TEST(GetActableUnitsExcludesWaitedAndDeadTest)

bool GetActableUnitsExcludesWaitedAndDeadTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FBaseUnit* a = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* b = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 2});
	FBaseUnit* c = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 3});
	TestsHelper::SpawnUnit(controller, ETeams::Enemy, {5, 5});

	std::vector<FBaseUnit*> actable;
	query->GetActableUnits(ETeams::Player, actable);
	TestEqual(TEXT("All three can act"), static_cast<int32>(actable.size()), 3);

	controller.WaitUnit(a);
	c->ReceiveDamage(1000);

	query->GetActableUnits(ETeams::Player, actable);
	TestEqual(TEXT("Only b remains"), static_cast<int32>(actable.size()), 1);
	TestTrue(TEXT("It is b"), !actable.empty() && actable[0] == b);

	query->GetActableUnits(ETeams::Enemy, actable);
	TestEqual(TEXT("Other team is unaffected"), static_cast<int32>(actable.size()), 1);
	return true;
}

BATTLEFIELD_FLOW_TEST(AllUnitsWaitedTest)

bool AllUnitsWaitedTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FBaseUnit* a = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* b = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 2});
	TestsHelper::SpawnUnit(controller, ETeams::Enemy, {5, 5});

	TestFalse(TEXT("Nobody waited"), query->AllUnitsWaited(ETeams::Player));
	controller.WaitUnit(a);
	TestFalse(TEXT("One of two waited"), query->AllUnitsWaited(ETeams::Player));
	controller.WaitUnit(b);
	TestTrue(TEXT("Both waited"), query->AllUnitsWaited(ETeams::Player));
	TestFalse(TEXT("Enemy team not affected"), query->AllUnitsWaited(ETeams::Enemy));
	return true;
}

BATTLEFIELD_FLOW_TEST(AllUnitsWaitedIgnoresDeadUnitsTest)

bool AllUnitsWaitedIgnoresDeadUnitsTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	FBattleController controller(&grid);
	const FBattleQueries* query = controller.GetQueryController();
	FBaseUnit* a = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 1});
	FBaseUnit* b = TestsHelper::SpawnUnit(controller, ETeams::Player, {1, 2});
	TestsHelper::SpawnUnit(controller, ETeams::Enemy, {5, 5});

	b->ReceiveDamage(1000);
	TestFalse(TEXT("Living unit has not waited"), query->AllUnitsWaited(ETeams::Player));
	controller.WaitUnit(a);
	TestTrue(TEXT("Dead unit does not count"), query->AllUnitsWaited(ETeams::Player));
	return true;
}

// --------------- Events Tests End
