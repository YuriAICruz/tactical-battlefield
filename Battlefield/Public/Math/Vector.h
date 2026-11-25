#pragma once
#include "Platform/CoreTypes.h"

// Minimal stand-ins for the Unreal math types used by FGridMath.
// Same names and field layout so the transpiled code and tests read the same.

struct FVector
{
	double X = 0.0;
	double Y = 0.0;
	double Z = 0.0;

	constexpr FVector() = default;

	constexpr FVector(double x, double y, double z) : X(x), Y(y), Z(z)
	{
	}

	constexpr FVector operator+(const FVector& other) const
	{
		return FVector(X + other.X, Y + other.Y, Z + other.Z);
	}

	constexpr FVector operator-(const FVector& other) const
	{
		return FVector(X - other.X, Y - other.Y, Z - other.Z);
	}

	constexpr FVector operator*(double scale) const
	{
		return FVector(X * scale, Y * scale, Z * scale);
	}

	constexpr FVector& operator+=(const FVector& other)
	{
		X += other.X;
		Y += other.Y;
		Z += other.Z;
		return *this;
	}

	constexpr bool operator==(const FVector& other) const
	{
		return X == other.X && Y == other.Y && Z == other.Z;
	}
};

struct FUintVector
{
	uint32 X = 0;
	uint32 Y = 0;
	uint32 Z = 0;

	constexpr FUintVector() = default;

	constexpr FUintVector(uint32 x, uint32 y, uint32 z) : X(x), Y(y), Z(z)
	{
	}
};

struct FMath
{
	static int32 FloorToInt(double value)
	{
		return static_cast<int32>(std::floor(value));
	}

	static bool IsNearlyEqual(double a, double b, double tolerance = 1.e-8)
	{
		return std::abs(a - b) <= tolerance;
	}
};
