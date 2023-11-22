#pragma once
#include "GridTile.h"
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
		std::vector<TileHandle>& GetTileHandles(bool) { return mGrid; }
		std::vector<Material>& GetMaterials() { return mMaterials; }
		u32 GetTileCount() { return static_cast<u32>(mTiles.size()); }
	private:
		void PlaceTiles();
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
		template <typename T>
		T getRandomInVector(const std::vector<T>& values) {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<std::size_t> distr(0, values.size() - 1);
			auto idx = distr(gen);
			return values[idx];
		}
	private:
		TileManager mTileManager;
		Location mStartLocation = { 0, 0 };
		std::vector<TileHandle> mGrid = std::vector<TileHandle>(GRID_DIM*GRID_DIM, {-1, 0, 0});
		std::vector<TileHandle> mAllValidTileHandles;
		std::vector<Tile> mTiles;
		std::vector<Material> mMaterials;
		std::vector<TileHandle> mTileHandles;
		std::unordered_map<id, std::unordered_set<id>> mSideToTiles;
	};
}