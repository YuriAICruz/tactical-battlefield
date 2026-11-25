#include "Core/Grid.h"
#include "Core/BaseUnit.h"
#include <algorithm>
#include <cassert>
#include <queue>

#include "Platform/BattlefieldLog.h"

static constexpr FGrid::FGridCoordinate kDirections[]
{
	{1, 0},
	{-1, 0},
	{0, 1},
	{0, -1}
};

FGrid::FGrid(uint16_t width, uint16_t height) :
	mWidth(width),
	mHeight(height)
{
	size_t tileCount = static_cast<size_t>(width) * static_cast<size_t>(height);

	mGCost.resize(tileCount);
	mDistance.resize(tileCount);
	mParents.resize(tileCount);
	mVisitedGeneration.resize(tileCount);
	mDiscoveredGeneration.resize(tileCount);
	mQueue.resize(tileCount);
	mActorPositions.reserve(tileCount);

	mTiles.reserve(static_cast<size_t>(width) * static_cast<size_t>(height));
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			mTiles.emplace_back(x, y);
		}
	}
}

uint16_t FGrid::GetWidth() const
{
	return mWidth;
}

uint16_t FGrid::GetHeight() const
{
	return mHeight;
}

bool FGrid::IsValid(int32 x, int32 y) const
{
	if (x < 0 || x >= mWidth)
		return false;

	if (y < 0 || y >= mHeight)
		return false;

	return true;
}

bool FGrid::IsValid(const FGridCoordinate& coord) const
{
	return IsValid(coord.x, coord.y);
}

bool FGrid::IsBlocked(int32 x, int32 y) const
{
	if (!IsValid(x, y))
	{
		return true;
	}

	const FTileData* tile = GetTile(x, y);
	return tile->IsBlocked();
}

bool FGrid::IsBlocked(const FGridCoordinate& coord) const
{
	return IsBlocked(coord.x, coord.y);
}

const FGrid::FTileData* FGrid::TryGetTile(int32 x, int32 y) const
{
	if (!IsValid(x, y))
		return nullptr;

	return &mTiles[GetIndex(x, y)];
}

const FGrid::FTileData* FGrid::TryGetTile(const FGridCoordinate& coord) const
{
	return TryGetTile(coord.x, coord.y);
}

FGrid::FTileData* FGrid::GetTile(int32 x, int32 y)
{
	assert(IsValid(x, y));
	return &mTiles[GetIndex(x, y)];
}

FGrid::FTileData* FGrid::GetTile(const FGridCoordinate& coord)
{
	return GetTile(coord.x, coord.y);
}

const FGrid::FTileData* FGrid::GetTile(int32 x, int32 y) const
{
	assert(IsValid(x, y));
	return &mTiles[GetIndex(x, y)];
}

const FGrid::FTileData* FGrid::GetTile(const FGridCoordinate& coord) const
{
	return GetTile(coord.x, coord.y);
}

size_t FGrid::TryGetIndex(int32 x, int32 y) const
{
	if (!IsValid(x, y))
	{
		return 0;
	}

	return GetIndex(x, y);
}

size_t FGrid::GetIndex(int32 x, int32 y) const
{
	assert(IsValid(x,y));
	return y * mWidth + x;
}

size_t FGrid::GetIndex(const FGridCoordinate& coord) const
{
	return GetIndex(coord.x, coord.y);
}

bool FGrid::IsVisited(uint32 Index) const
{
	return mVisitedGeneration[Index] == mCurrentGeneration;
}

void FGrid::AddGeneration() const
{
	//Reset if the generation overflow the uint32
	if (++mCurrentGeneration == 0)
	{
		std::ranges::fill(mVisitedGeneration, 0);
		std::ranges::fill(mDiscoveredGeneration, 0);
		mCurrentGeneration = 1;
	}
}

uint16 FGrid::Heuristic(const FGridCoordinate& A, const FGridCoordinate& B)
{
	return static_cast<uint16>(abs(A.x - B.x) + abs(A.y - B.y));
}

bool FGrid::IsDiscovered(uint32 index) const
{
	return mDiscoveredGeneration[index] == mCurrentGeneration;
}

template <typename TGoalPredicate, typename THeuristic>
bool FGrid::RunWeightedSearch(
	const FGridCoordinate& start,
	uint16 maxCost,
	TGoalPredicate&& goalPredicate,
	THeuristic&& heuristic,
	bool checkBlockedTiles) const
{
	AddGeneration();

	const uint32 startIndex = GetIndex(start);

	//mDiscoveredGeneration[startIndex] = mCurrentGeneration;
	mParents[startIndex] = -1;
	mGCost[startIndex] = 0;

	std::priority_queue<FPathNode, std::vector<FPathNode>, std::greater<FPathNode>> openSet;

	openSet.push({startIndex, heuristic(startIndex, 0)});

	while (!openSet.empty())
	{
		const FPathNode currentNode = openSet.top();
		openSet.pop();

		const uint32 currentIndex = currentNode.index;

		if (IsVisited(currentIndex))
		{
			continue;
		}

		mVisitedGeneration[currentIndex] = mCurrentGeneration;

		if (goalPredicate(currentIndex))
		{
			return true;
		}

		const FTileData& currentTile = mTiles[currentIndex];

		for (const FGridCoordinate& dir : kDirections)
		{
			FGridCoordinate neighbour{currentTile.coord.x + dir.x, currentTile.coord.y + dir.y};

			if (!IsValid(neighbour))
			{
				continue;
			}

			const uint32 neighbourIndex = GetIndex(neighbour);

			if (IsVisited(neighbourIndex))
			{
				continue;
			}

			const FTileData& neighbourTile = mTiles[neighbourIndex];

			if (checkBlockedTiles)
			{
				if (neighbourTile.IsBlocked())
				{
					continue;
				}
			}
			else
			{
				if (neighbourTile.IsWeightBlocked())
				{
					continue;
				}
			}

			const uint32 newCost = static_cast<uint32>(mGCost[currentIndex]) + 1u +
				static_cast<uint32>(neighbourTile.weight);

			if (newCost > maxCost)
			{
				continue;
			}

			const bool bFirstVisit = !IsDiscovered(neighbourIndex);

			if (bFirstVisit)
			{
				mDiscoveredGeneration[neighbourIndex] = mCurrentGeneration;
			}

			if (bFirstVisit || newCost < mGCost[neighbourIndex])
			{
				mGCost[neighbourIndex] = static_cast<uint16>(newCost);
				mParents[neighbourIndex] = static_cast<int32>(currentIndex);
				const uint32 priority = heuristic(neighbourIndex, newCost);
				openSet.push({neighbourIndex, static_cast<uint16>(priority)});
			}
		}
	}

	return false;
}

void FGrid::GetNeighbours(const FGridCoordinate& coord, std::vector<FGridCoordinate>& neighbours, uint16_t size,
                          bool bIncludeCenter) const
{
	neighbours.clear();
	neighbours.reserve(1 + 2 * size * (size + 1));

	if (!IsValid(coord))
	{
		return;
	}

	if (bIncludeCenter)
	{
		neighbours.push_back(coord);
	}

	if (size == 0)
	{
		return;
	}

	for (int y = -size; y <= size; ++y)
	{
		for (int x = -size; x <= size; ++x)
		{
			if (abs(x) + abs(y) > size)
			{
				continue;
			}
			if (x == 0 && y == 0)
			{
				continue;
			}

			FGridCoordinate neighbour
			{
				coord.x + x,
				coord.y + y
			};

			if (!IsValid(neighbour))
			{
				continue;
			}

			neighbours.push_back(neighbour);
		}
	}
}

void FGrid::GetWeightedNeighbours(const FGridCoordinate& coord, std::vector<FGridCoordinate>& neighbours,
                                  uint16 size, bool checkBlockedTiles) const
{
	neighbours.clear();

	if (!IsValid(coord))
	{
		return;
	}

	RunWeightedSearch(
		coord,
		size,
		[&](uint32 currentIndex)
		{
			neighbours.push_back(
				mTiles[currentIndex].coord);

			return false;
		},
		[&](uint32, uint32 gCost)
		{
			return gCost;
		},
		checkBlockedTiles);
}

int32 FGrid::GetDistance(FGridCoordinate origin, FGridCoordinate destination) const
{
	return Heuristic(origin, destination);
}

int32 FGrid::GetWeightedDistance(FGridCoordinate start, FGridCoordinate destination, bool checkBlockedTiles) const
{
	if (!IsValid(start) || !IsValid(destination))
	{
		return -1;
	}

	if (start == destination)
	{
		return 0;
	}

	const uint32 destinationIndex = GetIndex(destination);
	const bool bFound =
		RunWeightedSearch(
			start,
			INT16_MAX,
			[&](uint32 currentIndex)
			{
				return currentIndex == destinationIndex;
			},
			[&](uint32, uint32 gCost)
			{
				return gCost;
			},
			checkBlockedTiles);

	if (!bFound)
	{
		return -1;
	}

	return mGCost[destinationIndex];
}

bool FGrid::HasLineOfSight(const FGridCoordinate& start, const FGridCoordinate& end, bool checkBlockedTiles) const
{
	if (!IsValid(start) || !IsValid(end))
	{
		return false;
	}

	int32 x0 = start.x;
	int32 y0 = start.y;
	int32 x1 = end.x;
	int32 y1 = end.y;

	const int32 dx = std::abs(x1 - x0);
	const int32 dy = std::abs(y1 - y0);

	const int32 sx = (x0 < x1) ? 1 : -1;
	const int32 sy = (y0 < y1) ? 1 : -1;

	int32 err = dx - dy;

	while (true)
	{
		const FGridCoordinate current{x0, y0};

		if (current != start && current != end)
		{
			if (const FTileData* tile = GetTile(current))
			{
				if (checkBlockedTiles)
				{
					if (tile->IsBlocked())
					{
						return false;
					}
				}
				else
				{
					if (tile->IsWeightBlocked())
					{
						return false;
					}
				}
			}
		}

		if (x0 == x1 && y0 == y1)
		{
			break;
		}

		const int32 e2 = err * 2;

		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}

		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

void FGrid::GetVisibleTiles(const FGridCoordinate& origin, int32 range,
                            std::vector<FGridCoordinate>& visibleTiles, bool checkBlockedTiles) const
{
	visibleTiles.clear();

	for (int32 y = origin.y - range; y <= origin.y + range; ++y)
	{
		for (int32 x = origin.x - range; x <= origin.x + range; ++x)
		{
			FGridCoordinate target{x, y};

			if (!IsValid(target))
			{
				continue;
			}

			if (GetDistance(origin, target) > range)
			{
				continue;
			}

			if (HasLineOfSight(origin, target, checkBlockedTiles))
			{
				visibleTiles.push_back(target);
			}
		}
	}
}

bool FGrid::GetPath(const FGridCoordinate& start, const FGridCoordinate& end, uint16 size,
                    std::vector<FGridCoordinate>& path)
{
	path.clear();
	if (!IsValid(start))
	{
		return false;
	}
	if (!IsValid(end))
	{
		return false;
	}
	if (start == end)
	{
		path.push_back(start);
		return true;
	}

	AddGeneration();
	const uint32 startIndex = GetIndex(start);
	const uint32 endIndex = GetIndex(end);
	uint32 queueHead = 0;
	uint32 queueTail = 0;

	mQueue[queueTail++] = startIndex;

	mVisitedGeneration[startIndex] = mCurrentGeneration;
	mParents[startIndex] = -1;
	mDistance[startIndex] = 0;

	bool bFound = false;
	while (queueHead < queueTail)
	{
		const uint32 currentIndex = mQueue[queueHead++];
		if (currentIndex == endIndex)
		{
			bFound = true;
			break;
		}

		const uint16 currentDistance = mDistance[currentIndex];
		if (currentDistance >= size)
		{
			continue;
		}

		const FTileData& currentTile = mTiles[currentIndex];
		for (const FGridCoordinate& dir : kDirections)
		{
			FGridCoordinate neighbour
			{
				currentTile.coord.x + dir.x,
				currentTile.coord.y + dir.y
			};

			if (!IsValid(neighbour))
			{
				continue;
			}

			const uint32 neighbourIndex = GetIndex(neighbour);
			if (IsVisited(neighbourIndex))
			{
				continue;
			}

			const FTileData& neighbourTile = mTiles[neighbourIndex];
			if (neighbourTile.IsBlocked())
			{
				continue;
			}

			mVisitedGeneration[neighbourIndex] = mCurrentGeneration;
			mParents[neighbourIndex] = static_cast<int32>(currentIndex);
			mDistance[neighbourIndex] = currentDistance + 1;
			mQueue[queueTail++] = neighbourIndex;
		}
	}

	if (!bFound)
	{
		return false;
	}

	int32 current = static_cast<int32>(endIndex);
	while (current != -1)
	{
		path.push_back(mTiles[current].coord);
		current = mParents[current];
	}

	std::ranges::reverse(path);
	return true;
}

bool FGrid::GetWeightedPath(const FGridCoordinate& start, const FGridCoordinate& end,
                            std::vector<FGridCoordinate>& path,
                            uint16 size, bool checkBlockedTiles) const
{
	path.clear();

	if (!IsValid(start) || !IsValid(end))
	{
		return false;
	}

	if (start == end)
	{
		path.push_back(start);
		return true;
	}

	const uint32 endIndex = GetIndex(end);

	const bool bFound =
		RunWeightedSearch(
			start,
			size,
			[&](uint32 currentIndex)
			{
				return currentIndex == endIndex;
			},
			[&](uint32 index, uint32 gCost)
			{
				return gCost + Heuristic(mTiles[index].coord, end);
			},
			checkBlockedTiles);

	if (!bFound)
	{
		return false;
	}

	int32 current =
		static_cast<int32>(endIndex);

	while (current != -1)
	{
		path.push_back(mTiles[current].coord);
		current = mParents[current];
	}

	std::ranges::reverse(path);

	return true;
}

FGrid::FGridCoordinate FGrid::GetUnitPosition(const FGridEntity* actor) const
{
	if (actor == nullptr)
	{
		return {-1, -1};
	}

	auto it = mActorPositions.find(const_cast<FGridEntity*>(actor));
	if (it == mActorPositions.end())
	{
		BF_LOG(Error, "FGrid/GetUnitPosition: unit not found team[%d]", static_cast<int>(actor->GetTeam()));
		return {-1, -1};
	}
	return it->second;
}

void FGrid::GetUnitsOnArea(
	const std::vector<FGridCoordinate>& area, std::vector<const FGridEntity*>& units, ETeams filterMask) const
{
	units.clear();
	units.reserve(area.size());
	for (auto coord : area)
	{
		const FTileData* tile = GetTile(coord);
		if (tile->unit)
		{
			if (HasFlag(filterMask, tile->unit->GetTeam()))
			{
				units.push_back(tile->unit);
			}
		}
	}
}

bool FGrid::MoveUnitToTile(const FGridCoordinate& coord, FGridEntity* actor)
{
	if (actor == nullptr || !actor->CanMove())
	{
		BF_LOG(Error, "FGrid/MoveUnitToTile: invalid unit");
		return false;
	}

	if (!IsValid(coord))
	{
		BF_LOG(Error, "FGrid/MoveUnitToTile: invalid coord (%d, %d)", coord.x, coord.y);
		return false;
	}

	FTileData* tile = GetTile(coord);

	if (tile->unit == actor)
	{
		return true;
	}

	if (tile->IsBlocked())
	{
		BF_LOG(Error, "FGrid/MoveUnitToTile: invalid tile (%d, %d)", coord.x, coord.y);
		return false;
	}

	auto it = mActorPositions.find(const_cast<FGridEntity*>(actor));
	if (it != mActorPositions.end())
	{
		const FGridCoordinate& oldCoord = it->second;
		auto oldTile = GetTile(oldCoord);
		assert(oldTile->unit == actor);
		if (oldTile->unit != actor)
		{
			BF_LOG(Error,
				"FGrid/MoveUnitToTile: no actor on old position (%d, %d)", oldCoord.x, oldCoord.y);
			return false;
		}
		oldTile->unit = nullptr;
	}

	tile->unit = actor;
	mActorPositions[actor] = coord;
	return true;
}

void FGrid::RemoveUnitFromTile(const FGridEntity* target)
{
	FGridCoordinate pos = GetUnitPosition(target);
	FTileData* tile = GetTile(pos);
	assert(tile->unit == target);
	if (tile->unit == target)
	{
		tile->unit = nullptr;
	}
}

void FGrid::SetTileWeight(int32 x, int32 y, int8_t weight)
{
	if (!IsValid(x, y))
	{
		return;
	}

	GetTile(x, y)->weight = weight;
}
