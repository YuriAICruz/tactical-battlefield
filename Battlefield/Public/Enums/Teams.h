#pragma once
#include "Platform/CoreTypes.h"

enum class ETeams : uint8
{
	Null = 0,
	Player = 1 << 0,
	Enemy = 1 << 1,
	Neutral = 1 << 2,
	Length = 4
};

constexpr ETeams AllTeams = static_cast<ETeams>(
	static_cast<uint8_t>(ETeams::Player) | static_cast<uint8_t>(ETeams::Enemy) | static_cast<uint8_t>(ETeams::Neutral)
);

inline ETeams Invert(ETeams teams)
{
	return static_cast<ETeams>(
		~static_cast<uint8_t>(teams) &
		static_cast<uint8_t>(AllTeams)
	);
}

constexpr bool HasFlag(ETeams mask, ETeams flag)
{
	return (static_cast<uint8_t>(mask) & static_cast<uint8_t>(flag)) != 0;
}
