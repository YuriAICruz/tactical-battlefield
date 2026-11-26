#pragma once
#include "Platform/CoreTypes.h"
#include "Enums/Teams.h"
#include <unordered_map>
#include <vector>

class FGridEntity;

class BATTLEFIELD_API FGrid
{
public:
	struct FGridCoordinate
	{
		int32 x = 0;
		int32 y = 0;

		constexpr FGridCoordinate() = default;

		constexpr FGridCoordinate(int32 x, int32 y) : x(x), y(y)
		{
		}

		constexpr FGridCoordinate operator+(const FGridCoordinate& Other) const
		{
			return FGridCoordinate(x + Other.x, y + Other.y);
		}

		constexpr FGridCoordinate operator-(const FGridCoordinate& Other) const
		{
			return FGridCoordinate(x - Other.x, y - Other.y);
		}

		FGridCoordinate& operator+=(const FGridCoordinate& Other)
		{
			x += Other.x;
			y += Other.y;
			return *this;
		}

		FGridCoordinate& operator-=(const FGridCoordinate& Other)
		{
			x -= Other.x;
			y -= Other.y;
			return *this;
		}

		bool operator==(const FGridCoordinate& Other) const
		{
			return x == Other.x && y == Other.y;
		}

		bool operator!=(const FGridCoordinate& Other) const
		{
			return !(*this == Other);
		}
	};

	struct FTileData
	{
		FTileData(uint16_t x, uint16_t y) :
			coord(x, y),
			unit(nullptr)
		{
		}

		const FGridCoordinate coord;
		int8_t weight = 0;
		FGridEntity* unit;

		bool IsBlocked() const
		{
			return weight < 0 || unit != nullptr;
		}

		bool IsWeightBlocked() const
		{
			return weight < 0;
		}
	};

	static constexpr int8_t kImpassableWeight = -1;

	FGrid() = default;
	FGrid(uint16_t width, uint16_t height);
	uint16_t GetWidth() const;
	uint16_t GetHeight() const;
	bool IsValid(int32 x, int32 y) const;
	bool IsValid(const FGridCoordinate& coord) const;
	size_t TryGetIndex(int32 x, int32 y) const;
	bool IsBlocked(int32 x, int32 y) const;
	bool IsBlocked(const FGridCoordinate& coord) const;
	const FTileData* TryGetTile(int32 x, int32 y) const;
	const FTileData* TryGetTile(const FGridCoordinate& coord) const;
	void GetNeighbours(const FGridCoordinate& coord, std::vector<FGridCoordinate>& neighbours, uint16_t size = 1,
	                   bool bIncludeCenter = false) const;
	// passThroughTeam: when not Null (and checkBlockedTiles is set), units of that team can be crossed but the
	// search never ends on them. Null keeps every unit blocking.
	void GetWeightedNeighbours(const FGridCoordinate& coord, std::vector<FGridCoordinate>& neighbours,
	                           uint16 size, bool checkBlockedTiles, ETeams passThroughTeam = ETeams::Null) const;
	int32 GetDistance(FGridCoordinate origin, FGridCoordinate destination) const;
	int32 GetWeightedDistance(FGridCoordinate Start, FGridCoordinate Destination, bool checkBlockedTiles,
	                          ETeams passThroughTeam = ETeams::Null) const;
	bool HasLineOfSight(const FGridCoordinate& start, const FGridCoordinate& end, bool checkBlockedTiles) const;
	void GetVisibleTiles(const FGridCoordinate& origin, int32 range, std::vector<FGridCoordinate>& visibleTiles, bool checkBlockedTiles) const;
	bool GetPath(const FGridCoordinate& start, const FGridCoordinate& end, uint16 size,
	             std::vector<FGridCoordinate>& path);
	bool GetWeightedPath(const FGridCoordinate& start, const FGridCoordinate& end, std::vector<FGridCoordinate>& path,
	                     uint16 size, bool checkBlockedTiles) const;

	FGridCoordinate GetUnitPosition(const FGridEntity* actor) const;
	void GetUnitsOnArea(const std::vector<FGridCoordinate>& area, std::vector<const FGridEntity*>& units,
	                    ETeams filterMask) const;
	bool MoveUnitToTile(const FGridCoordinate& coord, FGridEntity* actor);
	void RemoveUnitFromTile(const FGridEntity* target);
	// Drops every unit from the grid (tiles and position map). Tile weights are kept.
	void ClearUnits();
	void SetTileWeight(int32 x, int32 y, int8_t weight);

private:
	struct FQueueNode
	{
		uint32 index;
		uint16 distance;
	};

	struct FPathNode
	{
		uint32 index;
		uint32 cost;

		bool operator>(const FPathNode& Other) const
		{
			return cost > Other.cost;
		}
	};

	std::vector<FTileData> mTiles;
	const uint16_t mWidth = 0;
	const uint16_t mHeight = 0;
	std::vector<uint16> mDistance;
	mutable uint32 mCurrentGeneration = 0;
	mutable std::vector<uint16> mGCost;
	mutable std::vector<int32> mParents;
	mutable std::vector<uint32> mVisitedGeneration;
	mutable std::vector<uint32> mDiscoveredGeneration;
	std::vector<uint32> mQueue;
	std::unordered_map<FGridEntity*, FGridCoordinate> mActorPositions;

	void AddGeneration() const;
	FTileData* GetTile(int32 x, int32 y);
	FTileData* GetTile(const FGridCoordinate& coord);
	const FTileData* GetTile(int32 x, int32 y) const;
	const FTileData* GetTile(const FGridCoordinate& coord) const;
	size_t GetIndex(int32 x, int32 y) const;
	size_t GetIndex(const FGridCoordinate& coord) const;
	bool IsVisited(uint32 Index) const;
	static uint16 Heuristic(const FGridCoordinate& A, const FGridCoordinate& B);
	bool IsDiscovered(uint32 index) const;

	template <class TGoalPredicate, class THeuristic>
	bool RunWeightedSearch(const FGridCoordinate& start, uint16 maxCost, TGoalPredicate&& goalPredicate,
	                       THeuristic&& heuristic, bool checkBlockedTiles = true,
	                       ETeams passThroughTeam = ETeams::Null) const;
};
