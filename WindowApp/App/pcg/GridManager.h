#pragma once
#include "GridTile.h"
#include "Grid.h"
#include "TileManager.h"
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <random>
#include <set>

namespace CPR::APP
{
	class GridManager
	{
	public:
		GridManager();
		~GridManager();
		void PlaceTile();
		void PlaceTiles();
		std::vector<Tile> GetTiles() { return _Grid.GetTiles(); }
		std::vector<TileHandle>& GetTileHandles(bool) { return mGrid; }
		std::vector<TileHandle>& GetTileHandles() { return _Grid.GetTileHandles(); }
		std::vector<Material>& GetMaterials() { return mMaterials; }
		template <typename T>
		T getRandomInVector(const std::vector<T>& values) {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<std::size_t> distr(0, values.size() - 1);
			auto idx = distr(gen);
			return values[idx];
		}
	private:
		void PlaceTiles(bool asd);
		std::vector<Location> GeneratePlacementOrder();
		std::vector<Location> NeighboringLocations(Location);
		std::vector<TileHandle> GenerateAllPossible();
		std::vector<TileHandle> GenerateSuperPositions(Location loc, i32 dir);
		std::vector<TileHandle> GenerateCompatibleTileHandles(i32 fromDir, i32 sideBetween);
		TileHandle TileHandleAt(Location loc);
		i32 OppositeDirection(i32 dir)
		{
			static constexpr i32 nrOfDirections = 4;
			return (dir + nrOfDirections / 2) % nrOfDirections;
		}
		i32 AdjacentSideIndex(i32 fromDir, i32 adjRot)
		{
			static constexpr i32 nrOfDirections = 4;
			return (nrOfDirections + fromDir - adjRot) % nrOfDirections;
		}
	private:
		Location mStartLocation = { 0, 0 };
		std::vector<TileHandle> mGrid = std::vector<TileHandle>(GRID_DIM*GRID_DIM, {-1, 0, 0});
		std::vector<TileHandle> mAllValidTileHandles;
		//std::unordered_map<Location, std::vector<TileHandle>> mLocToSuperPos;
		TileManager mTileManager;
		Grid _Grid;
		std::vector<Tile> mTiles;
		std::vector<Material> mMaterials;
		std::vector<TileHandle> mTileHandles;
		std::unordered_map<id, std::unordered_set<id>> mSideToTiles;
	};
}