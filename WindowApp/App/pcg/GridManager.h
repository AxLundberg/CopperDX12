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
		TileManager mTileManager;
		Grid _Grid;
		std::vector<Tile> mTiles;
		std::vector<Material> mMaterials;
		std::vector<TileHandle> mTileHandles;
		std::unordered_map<id, std::unordered_set<id>> mSideToTiles;
	};
}