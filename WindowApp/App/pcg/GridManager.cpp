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
		mTiles = _Grid.GetTiles();
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
}