#pragma once
#include "Platform/CoreTypes.h"

enum class EBattleAction : uint8
{
	None = 0,
	Move = 1 << 0,
	Attack = 1 << 1,
	DoActions = 1 << 2,
	Length = 4
};

constexpr EBattleAction AllAction = static_cast<EBattleAction>(
	static_cast<uint8_t>(EBattleAction::Move) | static_cast<uint8_t>(EBattleAction::Attack)
	| static_cast<uint8_t>(EBattleAction::DoActions)
);

inline EBattleAction InvertAction(EBattleAction teams)
{
	return static_cast<EBattleAction>(~static_cast<uint8_t>(teams) & static_cast<uint8_t>(AllAction));
}

constexpr bool ActionHasFlag(EBattleAction mask, EBattleAction flag)
{
	return (static_cast<uint8_t>(mask) & static_cast<uint8_t>(flag)) != 0;
}

constexpr void ActionAddFlag(EBattleAction& mask, EBattleAction flag)
{
	mask = static_cast<EBattleAction>(static_cast<uint8_t>(mask) | static_cast<uint8_t>(flag));
}
