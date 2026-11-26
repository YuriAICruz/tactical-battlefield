#include "Framework/AutomationShim.h"
#include "Core/BaseUnit.h"
#include "Enums/Teams.h"
#include "Core/TeamSystem.h"
#include "Core/Troupe.h"
#include "Core/StatusEffects/StatusEffect.h"
#include "Tests/TestsHelper.h"

static FTeamSystem CreateTestTeam()
{
	return FTeamSystem();
}

static FTroupe CreateTestTroupe(FTeamSystem& teamSystem)
{
	return FTroupe(&teamSystem);
}

static FBaseUnit* CreateTestActor(FTroupe& troupe, ETeams team)
{
	return troupe.CreateActor<FUnit>(TestsHelper::kBaseCombatStats, TestsHelper::kBaseTurnResources,
	                                     TestsHelper::kCombatWeapon, team);
}

#define TEAM_TEST(ClassName) IMPLEMENT_SIMPLE_AUTOMATION_TEST( \
ClassName, \
"Hellmarch.Battlefield.Team." #ClassName, \
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter )

TEAM_TEST(StartsFirstTest)

bool StartsFirstTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();

	TestEqual(TEXT("Player team should start"), TeamSystem.GetCurrentTeam(), ETeams::Player);

	return true;
}

TEAM_TEST(EndTurnSwitchesTeamTest)

bool EndTurnSwitchesTeamTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();

	TeamSystem.EndTurn();

	TestEqual(
		TEXT("Enemy turn"),
		TeamSystem.GetCurrentTeam(),
		ETeams::Enemy);

	return true;
}

TEAM_TEST(CycleTest)

bool CycleTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();

	for (int i = 0, n = static_cast<int>(ETeams::Length)-1; i < n; ++i)
	{
		TeamSystem.EndTurn();
	}

	TestEqual(TEXT("Back to player"), TeamSystem.GetCurrentTeam(), ETeams::Player);

	return true;
}

TEAM_TEST(FriendlyUnitsCanActSameTurnTest)

bool FriendlyUnitsCanActSameTurnTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* SoldierA = CreateTestActor(troupe, ETeams::Player);
	FBaseUnit* SoldierB = CreateTestActor(troupe, ETeams::Player);

	SoldierA->SetTeam(ETeams::Player);
	SoldierB->SetTeam(ETeams::Player);

	TestTrue(TEXT("Unit A can act"), TeamSystem.CanAct(SoldierA));
	TestTrue(TEXT("Unit B can act"), TeamSystem.CanAct(SoldierB));
	return true;
}

TEAM_TEST(EnemyCannotActDuringPlayerTurnTest)

bool EnemyCannotActDuringPlayerTurnTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* Enemy = CreateTestActor(troupe, ETeams::Enemy);

	TestFalse(TEXT("Enemy cannot act"), TeamSystem.CanAct(Enemy));
	return true;
}

TEAM_TEST(ExhaustedUnitDoesNotEndTeamTurnTest)

bool ExhaustedUnitDoesNotEndTeamTurnTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* A = CreateTestActor(troupe, ETeams::Player);
	FBaseUnit* B = CreateTestActor(troupe, ETeams::Player);

	A->SpendActionPoints(A->GetActionPoints());

	TestFalse(TEXT("A cannot act"), A->CanDoAction());
	TestTrue(TEXT("B can still act"), TeamSystem.CanAct(B));

	return true;
}

TEAM_TEST(EndTurnRefreshesTeamTest)

bool EndTurnRefreshesTeamTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* Soldier = CreateTestActor(troupe, ETeams::Player);

	Soldier->SpendActionPoints(2);
	Soldier->SpendMovementPoints(5);

	for (int i = 0; i < static_cast<int>(ETeams::Length); ++i)
	{
		TeamSystem.EndTurn();
	}

	TestEqual(TEXT("AP restored"), Soldier->GetActionPoints(), Soldier->GetMaxActionPoints());
	TestEqual(TEXT("Movement restored"), Soldier->GetMovementPoints(), Soldier->GetMaxMovementPoints());
	return true;
}

TEAM_TEST(DeadUnitsDoNotRefreshTest)

bool DeadUnitsDoNotRefreshTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* Soldier = CreateTestActor(troupe, ETeams::Player);

	Soldier->ReceiveDamage(TestsHelper::kBaseCombatStats.health);

	for (int i = 0; i < static_cast<int>(ETeams::Length); ++i)
	{
		TeamSystem.EndTurn();
	}

	TestFalse(TEXT("Still dead"), Soldier->IsAlive());

	return true;
}

TEAM_TEST(TeamDefeatedTest)

bool TeamDefeatedTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* A = CreateTestActor(troupe, ETeams::Enemy);
	FBaseUnit* B = CreateTestActor(troupe, ETeams::Enemy);

	A->ReceiveDamage(TestsHelper::kBaseCombatStats.health);
	B->ReceiveDamage(TestsHelper::kBaseCombatStats.health);

	TestTrue(TEXT("Enemy defeated"), TeamSystem.IsTeamDefeated(ETeams::Enemy));
	return true;
}

TEAM_TEST(TeamNotDefeatedIfOneUnitLivesTest)

bool TeamNotDefeatedIfOneUnitLivesTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* A = CreateTestActor(troupe, ETeams::Enemy);
	FBaseUnit* B = CreateTestActor(troupe, ETeams::Enemy);

	A->ReceiveDamage(TestsHelper::kBaseCombatStats.health);

	TestFalse(TEXT("Enemy still alive"), TeamSystem.IsTeamDefeated(ETeams::Enemy));
	return true;
}

TEAM_TEST(PlayerVictoryTest)

bool PlayerVictoryTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* Player = CreateTestActor(troupe, ETeams::Player);
	FBaseUnit* Enemy = CreateTestActor(troupe, ETeams::Enemy);

	Enemy->ReceiveDamage(TestsHelper::kBaseCombatStats.health);

	TestTrue(TEXT("Player wins"), TeamSystem.HasWinner());
	TestEqual(TEXT("Winner"), TeamSystem.GetWinner(), ETeams::Player);

	return true;
}

TEAM_TEST(PoisonTicksOnTurnStartTest)

bool PoisonTicksOnTurnStartTest::RunTest(const FString& Parameters)
{
	FTeamSystem TeamSystem = CreateTestTeam();
	FTroupe troupe = CreateTestTroupe(TeamSystem);
	FBaseUnit* Soldier = CreateTestActor(troupe, ETeams::Player);

	Soldier->SetTeam(ETeams::Player);

	FBaseStatusEffect* effect = Soldier->AddStatusEffect(1);

	for (int i = 0; i < static_cast<int>(ETeams::Length); ++i)
	{
		TeamSystem.EndTurn();
	}

	TestEqual(TEXT("Stun tick count"), effect->GetTicksApplied(), 1);
	return true;
}

TEAM_TEST(MutualDestructionHasNoWinnerTest)

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

TEAM_TEST(EnemyVictoryTest)

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
