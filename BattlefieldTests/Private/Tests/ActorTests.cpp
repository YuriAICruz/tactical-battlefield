#include "Framework/AutomationShim.h"
#include "Core/BaseUnit.h"
#include "Core/StatusEffects/StatusEffect.h"
#include "Tests/TestsHelper.h"
#include <memory>

static std::unique_ptr<FBaseUnit> CreateTestActor()
{
	return std::make_unique<FUnit>(0, TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                               TestsHelper::kCombatWeapon, ETeams::Null);
}

#define ACTOR_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Actor." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )

ACTOR_TEST(PositionTest)

bool PositionTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	std::unique_ptr<FBaseUnit> actor = CreateTestActor();

	grid.MoveUnitToTile({1, 1}, actor.get());
	FGrid::FGridCoordinate Position = grid.GetUnitPosition(actor.get());

	TestEqual(TEXT("X equals to 1"), Position.x, 1);
	TestEqual(TEXT("Y equals to 1"), Position.y, 1);

	return true;
}

ACTOR_TEST(DefaultPositionTest)

bool DefaultPositionTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();
	std::unique_ptr<FBaseUnit> actor = CreateTestActor();

	auto position = grid.GetUnitPosition(actor.get());

	TestEqual(TEXT("Default X"), position.x, -1);
	TestEqual(TEXT("Default Y"), position.y, -1);

	return true;
}

ACTOR_TEST(MoveActorToEmptyTileTest)

bool MoveActorToEmptyTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> actor = CreateTestActor();

	bool bMoved = grid.MoveUnitToTile({2, 3}, actor.get());

	TestTrue(TEXT("Move should succeed"), bMoved);

	auto Position = grid.GetUnitPosition(actor.get());

	TestEqual(TEXT("X"), Position.x, 2);
	TestEqual(TEXT("Y"), Position.y, 3);

	return true;
}

ACTOR_TEST(MoveActorUpdatesOccupancyTest)

bool MoveActorUpdatesOccupancyTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	grid.MoveUnitToTile({4, 5}, Actor.get());

	auto Tile = grid.TryGetTile(4, 5);

	TestEqual(TEXT("Tile occupant"), *Tile->unit, *static_cast<FGridEntity*>(Actor.get()));

	return true;
}

ACTOR_TEST(MoveActorClearsPreviousTileTest)

bool MoveActorClearsPreviousTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	grid.MoveUnitToTile({1, 1}, Actor.get());
	grid.MoveUnitToTile({2, 1}, Actor.get());

	auto OldTile = grid.TryGetTile(1, 1);
	auto NewTile = grid.TryGetTile(2, 1);

	TestNull(TEXT("Old tile should be empty"), OldTile->unit);

	TestEqual(TEXT("New tile occupant"), *NewTile->unit, *static_cast<FGridEntity*>(Actor.get()));

	return true;
}

ACTOR_TEST(MoveActorToOccupiedTileTest)

bool MoveActorToOccupiedTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> A = CreateTestActor();
	std::unique_ptr<FBaseUnit> B = CreateTestActor();

	grid.MoveUnitToTile({3, 3}, A.get());

	bool bMoved = grid.MoveUnitToTile({3, 3}, B.get());

	TestFalse(TEXT("Move should fail"), bMoved);

	return true;
}

ACTOR_TEST(FailedMovePreservesPositionTest)

bool FailedMovePreservesPositionTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> A = CreateTestActor();
	std::unique_ptr<FBaseUnit> B = CreateTestActor();

	grid.MoveUnitToTile({2, 2}, A.get());
	grid.MoveUnitToTile({1, 1}, B.get());

	bool bMoved = grid.MoveUnitToTile({2, 2}, B.get());

	TestFalse(TEXT("Move should fail"), bMoved);

	auto Position = grid.GetUnitPosition(B.get());

	TestEqual(TEXT("X"), Position.x, 1);
	TestEqual(TEXT("Y"), Position.y, 1);

	return true;
}

ACTOR_TEST(MoveActorToInvalidTileTest)

bool MoveActorToInvalidTileTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	bool bMoved = grid.MoveUnitToTile({999, 999}, Actor.get());

	TestFalse(TEXT("Move should fail"), bMoved);

	return true;
}

ACTOR_TEST(MoveNullActorTest)

bool MoveNullActorTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	bool bMoved = grid.MoveUnitToTile({1, 1}, nullptr);

	TestFalse(TEXT("Move should fail"), bMoved);

	return true;
}

ACTOR_TEST(ActorMultipleMovesTest)

bool ActorMultipleMovesTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> actor = CreateTestActor();

	grid.MoveUnitToTile({1, 1}, actor.get());
	grid.MoveUnitToTile({2, 1}, actor.get());
	grid.MoveUnitToTile({3, 1}, actor.get());
	grid.MoveUnitToTile({4, 1}, actor.get());

	FGrid::FGridCoordinate position = grid.GetUnitPosition(actor.get());

	TestEqual(TEXT("X"), position.x, 4);
	TestEqual(TEXT("Y"), position.y, 1);

	return true;
}

ACTOR_TEST(ActorBlocksPathfindingTest)

bool ActorBlocksPathfindingTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> Blocker = CreateTestActor();

	grid.MoveUnitToTile({2, 1}, Blocker.get());

	std::vector<FGrid::FGridCoordinate> Path;

	grid.GetPath({1, 1}, {3, 1}, 10, Path);

	bool bContainsBlocker =
		std::find(Path.begin(), Path.end(), FGrid::FGridCoordinate(2, 1)) != Path.end();

	TestFalse(TEXT("Path should avoid actor"), bContainsBlocker);

	return true;
}

ACTOR_TEST(ActorBlocksReachabilityTest)

bool ActorBlocksReachabilityTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> Blocker = CreateTestActor();

	grid.MoveUnitToTile({6, 5}, Blocker.get());

	std::vector<FGrid::FGridCoordinate> Reachable;

	grid.GetWeightedNeighbours({5, 5}, Reachable, 1, true);

	TestFalse(TEXT("Blocked tile excluded"), TestsHelper::ContainsTile(Reachable, {6, 5}));

	return true;
}

ACTOR_TEST(DefaultTeamTest)

bool DefaultTeamTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	TestEqual(TEXT("Default team"), Actor->GetTeam(), ETeams::Null);
	return true;
}

ACTOR_TEST(SetTeamTest)

bool SetTeamTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	Actor->SetTeam(ETeams::Player);
	TestEqual(TEXT("Player team"), Actor->GetTeam(), ETeams::Player);
	return true;
}

ACTOR_TEST(EnemyDetectionTest)

bool EnemyDetectionTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> A = CreateTestActor();
	std::unique_ptr<FBaseUnit> B = CreateTestActor();

	A->SetTeam(ETeams::Player);
	B->SetTeam(ETeams::Enemy);
	TestTrue(TEXT("Should be enemies"), A->IsEnemy(B.get()));

	return true;
}

ACTOR_TEST(FriendlyDetectionTest)

bool FriendlyDetectionTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> A = CreateTestActor();
	std::unique_ptr<FBaseUnit> B = CreateTestActor();

	A->SetTeam(ETeams::Player);
	B->SetTeam(ETeams::Player);
	TestFalse(TEXT("Should not be enemies"), A->IsEnemy(B.get()));

	return true;
}

ACTOR_TEST(NullHostileDetectionTest)

bool NullHostileDetectionTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> A = CreateTestActor();
	std::unique_ptr<FBaseUnit> B = CreateTestActor();

	A->SetTeam(ETeams::Null);
	B->SetTeam(ETeams::Player);
	TestFalse(TEXT("Should not be enemies"), A->IsEnemy(B.get()));

	return true;
}

ACTOR_TEST(ActorStartsAliveTest)

bool ActorStartsAliveTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	TestTrue(TEXT("Actor starts alive"), Actor->IsAlive());

	return true;
}

ACTOR_TEST(KillActorTest)

bool KillActorTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	int bDeathEventTriggered = 0;
	Actor->OnDeath = [&](const FBaseUnit* target)
	{
		bDeathEventTriggered++;
	};

	Actor->ReceiveDamage(TestsHelper::kBaseCombatStats.maxHealth);
	Actor->ReceiveDamage(TestsHelper::kBaseCombatStats.maxHealth);

	TestFalse(TEXT("Actor should be dead"), Actor->IsAlive());
	TestEqual(TEXT("Death event should be triggered"), bDeathEventTriggered, 1);

	return true;
}


ACTOR_TEST(HealActorTest)

bool HealActorTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	Actor->ReceiveDamage(TestsHelper::kBaseCombatStats.maxHealth / 2);

	TestTrue(TEXT("Actor should not be dead"), Actor->IsAlive());
	TestEqual(TEXT("Actor should be damaged"), Actor->GetHealth(),
	          TestsHelper::kBaseCombatStats.maxHealth - (TestsHelper::kBaseCombatStats.maxHealth / 2));

	Actor->ReceiveHeal(TestsHelper::kBaseCombatStats.maxHealth);

	TestTrue(TEXT("Actor should not be dead"), Actor->IsAlive());
	TestEqual(TEXT("Actor should be healed"), Actor->GetHealth(), TestsHelper::kBaseCombatStats.maxHealth);

	return true;
}

ACTOR_TEST(CanMoveTest)

bool CanMoveTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	TestTrue(TEXT("Actor should be able to move"), Actor->CanMove());
	Actor->SpendMovementPoints(TestsHelper::kBaseTurnResources.move);

	TestTrue(TEXT("Actor should not dead"), Actor->IsAlive());
	TestFalse(TEXT("Actor should not be able to move"), Actor->CanMove());

	return true;
}

ACTOR_TEST(CanDoActionTest)

bool CanDoActionTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	TestTrue(TEXT("Actor should be able to do actions"), Actor->CanDoAction());
	Actor->SpendActionPoints(TestsHelper::kBaseTurnResources.ap);

	TestTrue(TEXT("Actor should not dead"), Actor->IsAlive());
	TestFalse(TEXT("Actor should not be able to do actions"), Actor->CanDoAction());

	return true;
}

ACTOR_TEST(DeadActorCannotMoveTest)

bool DeadActorCannotMoveTest::RunTest(const FString& Parameters)
{
	FGrid grid = TestsHelper::CreateTestGrid();

	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	Actor->ReceiveDamage(TestsHelper::kBaseCombatStats.maxHealth);

	bool bMoved = grid.MoveUnitToTile({2, 2}, Actor.get());

	TestFalse(TEXT("Dead actor cannot move"), bMoved);

	return true;
}

ACTOR_TEST(MovementPointsDefaultTest)

bool MovementPointsDefaultTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	TestEqual(TEXT("Default movement"), Actor->GetMovementPoints(), TestsHelper::kBaseTurnResources.move);
	return true;
}

ACTOR_TEST(SpendMovementPointsTest)

bool SpendMovementPointsTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	bool bSuccess = Actor->SpendMovementPoints(3);

	TestTrue(TEXT("Spend succeeds"), bSuccess);
	TestEqual(TEXT("Remaining movement"), Actor->GetMovementPoints(), TestsHelper::kBaseTurnResources.move - 3);

	return true;
}

ACTOR_TEST(CannotOverspendMovementTest)

bool CannotOverspendMovementTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	bool bSuccess = Actor->SpendMovementPoints(999);

	TestFalse(TEXT("Cannot overspend"), bSuccess);

	return true;
}

ACTOR_TEST(ResetMovementPointsTest)

bool ResetMovementPointsTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	Actor->SpendMovementPoints(3);

	Actor->ResetTurnStats();

	TestEqual(TEXT("Movement reset"), Actor->GetMovementPoints(), Actor->GetMaxMovementPoints());

	return true;
}

ACTOR_TEST(ActionPointsDefaultTest)

bool ActionPointsDefaultTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	TestEqual(TEXT("Default AP"), Actor->GetActionPoints(), TestsHelper::kBaseTurnResources.ap);
	return true;
}

ACTOR_TEST(SpendActionPointTest)

bool SpendActionPointTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> actor = CreateTestActor();
	actor->SpendActionPoints(1);
	TestEqual(TEXT("Remaining AP"), actor->GetActionPoints(), TestsHelper::kBaseTurnResources.ap - 1);

	return true;
}

ACTOR_TEST(CannotUseActionWhenOutOfAPTest)

bool CannotUseActionWhenOutOfAPTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	Actor->SpendActionPoints(TestsHelper::kBaseTurnResources.ap);
	TestFalse(TEXT("Cannot spend more AP"), Actor->SpendActionPoints(1));
	return true;
}

ACTOR_TEST(AddStatusEffectTest)

bool AddStatusEffectTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	const FBaseStatusEffect* effect = Actor->AddStatusEffect(1);
	TestTrue(TEXT("Has poison"), Actor->HasStatusType(1));
	return true;
}

ACTOR_TEST(RemoveStatusEffectTest)

bool RemoveStatusEffectTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	const FBaseStatusEffect* effect = Actor->AddStatusEffect(1);
	TestTrue(TEXT("Has poison"), Actor->HasStatusType(1));
	Actor->RemoveStatusEffect(1);
	TestFalse(TEXT("Poison removed"), Actor->HasStatusType(1));
	return true;
}

ACTOR_TEST(DuplicateStatusTest)

bool DuplicateStatusTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	const FBaseStatusEffect* effect = Actor->AddStatusEffect(1);
	TestTrue(TEXT("Poison is True"), effect != nullptr);
	const FBaseStatusEffect* effect2 = Actor->AddStatusEffect(1);
	TestFalse(TEXT("Second Poison is False"), effect2 != nullptr);

	TestTrue(TEXT("Has poison"), Actor->HasStatusType(1));

	return true;
}

ACTOR_TEST(StunnedBlocksActionsTest)

bool StunnedBlocksActionsTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();
	const FBaseStatusEffect* effect = Actor->AddStatusEffect(1);

	Actor->ResetTurnStats();
	TestFalse(TEXT("Cannot act"), Actor->CanDoAction());

	return true;
}

ACTOR_TEST(StatusAddedEventTest)

bool StatusAddedEventTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	bool bTriggered = false;

	Actor->OnStatusAdded =
		[&](const FBaseUnit* Unit, uint8 Effect)
		{
			bTriggered = true;

			TestEqual(TEXT("Correct Unit"), Unit, static_cast<const FBaseUnit*>(Actor.get()));
			TestEqual(TEXT("Correct Status"), Effect, 1);
		};

	Actor->AddStatusEffect(1);

	TestTrue(TEXT("Event triggered"), bTriggered);

	return true;
}

ACTOR_TEST(StatusRemovedEventTest)

bool StatusRemovedEventTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	Actor->AddStatusEffect(1);

	bool bTriggered = false;

	Actor->OnStatusRemoved =
		[&](const FBaseUnit* Unit, uint8 Effect)
		{
			bTriggered = true;

			TestEqual(TEXT("Correct Unit"), Unit, static_cast<const FBaseUnit*>(Actor.get()));
			TestEqual(TEXT("Correct Status"), Effect, 1);
		};

	Actor->RemoveStatusEffect(1);

	TestTrue(TEXT("Event triggered"), bTriggered);

	return true;
}

ACTOR_TEST(StatusTriggeredEventTest)

bool StatusTriggeredEventTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	Actor->AddStatusEffect(1);

	bool bTriggered = false;

	Actor->OnStatusTriggered =
		[&](const FBaseUnit* Unit, uint8 Effect)
		{
			bTriggered = true;

			TestEqual(TEXT("Correct Unit"), Unit, static_cast<const FBaseUnit*>(Actor.get()));
			TestEqual(TEXT("Correct Status"), Effect, 1);
		};

	Actor->ResetTurnStats();

	TestTrue(TEXT("Event triggered"), bTriggered);

	return true;
}

ACTOR_TEST(DuplicateStatusDoesNotBroadcastTest)

bool DuplicateStatusDoesNotBroadcastTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	int32 AddedCount = 0;

	Actor->OnStatusAdded =
		[&](const FBaseUnit*, uint8)
		{
			AddedCount++;
		};

	Actor->AddStatusEffect(1);
	Actor->AddStatusEffect(1);

	TestEqual(TEXT("Added only once"), AddedCount, 1);

	return true;
}

ACTOR_TEST(StatusAddRemoveOrderTest)

bool StatusAddRemoveOrderTest::RunTest(const FString& Parameters)
{
	std::unique_ptr<FBaseUnit> Actor = CreateTestActor();

	std::vector<FString> Events;

	Actor->OnStatusAdded =
		[&](const FBaseUnit*, uint8)
		{
			Events.push_back(TEXT("Added"));
		};

	Actor->OnStatusRemoved =
		[&](const FBaseUnit*, uint8)
		{
			Events.push_back(TEXT("Removed"));
		};

	Actor->AddStatusEffect(1);
	Actor->RemoveStatusEffect(1);

	TestEqual(TEXT("Event count"), static_cast<int32>(Events.size()), 2);
	TestEqual(TEXT("First"), Events[0], FString(TEXT("Added")));
	TestEqual(TEXT("Second"), Events[1], FString(TEXT("Removed")));

	return true;
}
