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


// --------------- Events Tests End
