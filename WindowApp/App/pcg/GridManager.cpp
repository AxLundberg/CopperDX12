#include "GridManager.h"
#include "TileData.h"
#include <iterator>
#include <ranges>
#include <random>

namespace CPR::APP
{
	namespace rn = std::ranges;
	namespace vi = rn::views;

	GridManager::~GridManager() {}
	
	GridManager::GridManager()
		: _Grid(GRID_DIM)
	{
		mTiles = mTileManager.CreateTiles(PATH);
		mMaterials = mTileManager.CreateMaterials();
		mSideToTiles = mTileManager.CreateAdjacencyMap(mTiles);
		mTileHandles = _Grid.GetTileHandles();
		for (i32 i = 1; i < mTiles.size(); i++)
			for (i32 j = 0; j < NR_OF_DIRECTIONS; j++)
				mAllValidTileHandles.push_back(TileHandle{ i, j, 0 });
		GeneratePlacementOrder();
		PlaceTiles(true);
	}

	void GridManager::PlaceTiles()
	{
		static bool first = true;
		// place start tile
		if (first)
		{
			auto startTile = _Grid.GetStartPos();
			std::vector<int> startTileIds = { 5 };
			i32 tileID = getRandomInVector<i32>(startTileIds);
			_Grid.PlaceTile(startTile, { tileID, 0, false });
			first = false;
		}
		// place remaining tiles
		for (u32 y = 0; y < GRID_DIM; y++)
		{
			for (u32 x = 0; x < GRID_DIM; x++)
			{
				if (y == 0 && x == 0)
					continue;
				auto& frontier = _Grid.GetFrontier();
				auto nextLoc = getRandomInVector<location>(frontier);
				if (x == 1 && y == 0) nextLoc = { 0, 1 };
				if (x == 2 && y == 0) nextLoc = { 1, 1 };
				auto& locSuperPos = _Grid.GetSuperPositions(nextLoc);
				auto tileToPlace = locSuperPos.size() ?
					getRandomInVector<TileHandle>(locSuperPos) :
					TileHandle{ .id = 0, .rotation = 0, .reflect = 0 };
				if (tileToPlace.id == 0)
					auto stop = 0;
				_Grid.PlaceTile(nextLoc, tileToPlace);
			}
		}
	}

	void GridManager::PlaceTile()
	{
		static bool first = true;
		// place start tile
		if (first)
		{
			auto startTile = _Grid.GetStartPos();
			std::vector<int> startTileIds = { 5 };
			i32 tileID = getRandomInVector<i32>(startTileIds);
			_Grid.PlaceTile(startTile, { tileID, 0, false });
			first = false;
		}
		// place next tile
		else
		{
			auto& frontier = _Grid.GetFrontier();
			auto nextLoc = getRandomInVector<location>(frontier);
			auto& locSuperPos = _Grid.GetSuperPositions(nextLoc);
			auto tileToPlace = locSuperPos.size() ?
				getRandomInVector<TileHandle>(locSuperPos) :
				TileHandle{ .id = 0, .rotation = 0, .reflect = 0 };
			if (tileToPlace.id == 0)
				auto stop = 0;
			_Grid.PlaceTile(nextLoc, tileToPlace);
		}
	}
	
	static constexpr bool OutOfBounds(Location l)
	{
		return l.x < 0 || l.x >= GRID_DIM || l.y < 0 || l.y >= GRID_DIM;
	}
	static constexpr i32 to1D(Location l)
	{
		return l.x + l.y * static_cast<i32>(GRID_DIM);
	}
	static constexpr i32 AdjacentSideIndex(i32 fromDir, i32 adjRot)
	{
		return (NR_OF_DIRECTIONS + fromDir - adjRot) % NR_OF_DIRECTIONS;
	}
	static constexpr i32 OppositeDirection(i32 dir)
	{
		return (dir + NR_OF_DIRECTIONS / 2) % NR_OF_DIRECTIONS;
	}
	TileHandle GridManager::TileHandleAt(Location loc)
	{
		return mGrid[to1D(loc)];
	}
	void GridManager::PlaceTiles(bool sda)
	{
		auto placementOrder = GeneratePlacementOrder();

		// place first tile separately
		std::vector<int> startTileIds = { 5 };
		i32 tileID = getRandomInVector<i32>(startTileIds);
		mGrid[0].id = tileID;

		// place rest of tiles
		for (u32 i = 1; i < placementOrder.size(); i++)
		{
			auto& loc = placementOrder[i];

			// Generate possible superposition based on neighboring tiles
			auto sprPosNorth = GenerateSuperPositions(loc, NORTH);
			auto sprPosEast = GenerateSuperPositions(loc, EAST);
			auto sprPosSouth = GenerateSuperPositions(loc, SOUTH);
			auto sprPosWest = GenerateSuperPositions(loc, WEST);

			std::vector<TileHandle> temp1, temp2, result;

			// Store intersection of the superpositions in temporary vectors
			std::set_intersection(sprPosNorth.begin(), sprPosNorth.end(), sprPosEast.begin(), sprPosEast.end(), std::back_inserter(temp1));
			std::set_intersection(sprPosWest.begin(), sprPosWest.end(), sprPosSouth.begin(), sprPosSouth.end(), std::back_inserter(temp2));

			// Intersection of the two temp vectors give the final intersection of super positions
			std::set_intersection(temp1.begin(), temp1.end(), temp2.begin(), temp2.end(), std::back_inserter(result));

			// If there are valid superpositions, pick a random one and place it in grid
			if (result.size())
			{
				auto placed = getRandomInVector(result);
				mGrid[loc.x + loc.y * GRID_DIM] = placed;
			}
		}
	}
	std::vector<TileHandle> GridManager::GenerateSuperPositions(Location loc, i32 dir)
	{
		std::vector<TileHandle> superPositions;
		
		auto adjacentLoc = loc.NeighborAt(dir);
		if (OutOfBounds(adjacentLoc))
			return mAllValidTileHandles;

		auto adjacentHandle = TileHandleAt(adjacentLoc);
		if(adjacentHandle.id == FREE)
			return mAllValidTileHandles;

		Tile adjacentTile = mTiles[adjacentHandle.id];
		auto adjRot = adjacentHandle.rotation;
		auto sideIdBetween = adjacentTile.GetSideID(dir, adjRot);

		if (sideIdBetween == 0) // Set all as compatible
			superPositions = mAllValidTileHandles;
		else
			superPositions = GenerateCompatibleTileHandles(dir, sideIdBetween);
		
		auto bck = superPositions.back();
		auto stop = 45;
		std::sort(superPositions.begin(), superPositions.end());
		return superPositions;
	}

	std::vector<TileHandle> GridManager::GenerateCompatibleTileHandles(i32 fromDir, i32 sideBetween)
	{
		auto oppositeDir = OppositeDirection(fromDir);
		auto compatibleTiles = std::vector<TileHandle>();
		auto& tiles = mSideToTiles[sideBetween];

		for (auto tileID : tiles)
		{
			if (sideBetween == 0) // ??? ? ?? ? 
				break;
			auto& tile = mTiles[tileID];
			for (auto dir : DIRECTION)
			{
				auto tsID = tile.sideIDs[dir];

				bool hasReflextion = mSideToTiles.find(tsID) != mSideToTiles.end() &&
					mSideToTiles.find(-tsID) != mSideToTiles.end();
				if (!(tsID == -sideBetween || (!hasReflextion && tsID == sideBetween)))
				{
					continue; // TODO: reflexion
				}
				i32 reqRot = (fromDir - dir) < 0 ? (fromDir - dir) + 4 : (fromDir - dir);
				compatibleTiles.push_back(TileHandle{
					.id = tileID,
					.rotation = reqRot,
					.reflect = 0
					});
			}
		}
		return compatibleTiles;
	}
	std::vector<Location> GridManager::GeneratePlacementOrder()
	{
		// lambda function for union of Location vectors
		auto unionize = [](std::vector<Location>& v1, const std::vector<Location>& v2) {
			// Append v2 to v1
			v1.insert(v1.end(), v2.begin(), v2.end());
			// Sort v1
			std::sort(v1.begin(), v1.end(), [](const Location& a, const Location& b) {
				return (a.x < b.x) || (a.x == b.x && a.y < b.y);
				});
			// Remove duplicates in v1
			v1.erase(std::unique(v1.begin(), v1.end()), v1.end());
		};

		std::vector<Location> placementOrder;
		std::vector<Location> frontier;
		placementOrder.push_back(mStartLocation);
		for (u32 i = 1; i < GRID_DIM * GRID_DIM; i++)
		{
			// get new frontier locations
			unionize(frontier, NeighboringLocations(placementOrder.back()));
			// remove frontier locations that have already been placed
			for (auto it = frontier.begin(); it != frontier.end(); ) {
				if (std::find(placementOrder.begin(), placementOrder.end(), *it) != placementOrder.end())
					it = frontier.erase(it);
				else
					++it;
			}
			placementOrder.push_back(getRandomInVector(frontier));
		}
		return placementOrder;
	}

	

	std::vector<Location> GridManager::NeighboringLocations(Location loc)
	{
		std::vector<Location> neighbors;
		for (i32 i = 0; i < NR_OF_DIRECTIONS; i++)
		{
			auto neighbor = loc.NeighborAt(i);
			if (!OutOfBounds(neighbor))
				neighbors.push_back(neighbor);
		}
		return neighbors;
	}

	std::vector<TileHandle> GridManager::GenerateAllPossible()
	{
		std::vector<TileHandle> allPossibleTileHandles;
		auto& tiles = mSideToTiles[0];
		for (auto tileID : tiles)
		{
			auto& tile = mTiles[tileID];
			for (auto dir : DIRECTION)
			{
				allPossibleTileHandles.push_back(TileHandle{
					.id = tileID,
					.rotation = dir,
					.reflect = 0
					});
			}
		}
		std::sort(allPossibleTileHandles.begin(), allPossibleTileHandles.end());
		return allPossibleTileHandles;
	}

}