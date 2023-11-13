#pragma once
#include "GridTile.h"
#include "Tile.h"
#include "TileManager.h"
#include <vector>
#include <optional>

namespace CPR::APP
{
	static constexpr TileHandle ANY_TILE = { -1, -1, -1 };
	static constexpr TileHandle RED_TILE = { 0, 0, 0 };
	static constexpr id RED_TILE_ID = 0;
	class Grid
	{
	public:
		Grid(u32 dim);
		~Grid();
		location GetStartPos();
		void PlaceTile(location loc, TileHandle tileHandle);
		std::vector<location>& GetFrontier();
		std::vector<TileHandle>& GetSuperPositions(location loc);
		std::vector<Tile> GetTiles() { return mTiles; }
		std::vector<TileHandle>& GetTileHandles() { return mTileHandles; }
	private:
		TileHandle* At(location);
		std::optional<location> LocAt(u32 dir, location loc);
		std::vector<TileHandle> Intersect(std::vector<TileHandle>& a, std::vector<TileHandle>& b);
		void RemoveFromFrontier(location toRemove);
		void AddNeighborsToFrontier(location loc);
		void UpdateSuperPositions(location loc, i32 fromDir);
		void UpdateFrontierNeighbors(location loc);

	private:
		u32 dim;
		TileManager mTileManager;
		location mStartPos;
		std::vector<Tile> mTiles;
		std::vector<location> mFrontier;
		std::vector<TileHandle> mTileHandles;
		std::unordered_map<location, std::vector<TileHandle>, pair_hash> mLocToSuperPositions;
		std::unordered_map<id, std::unordered_set<id>> mSideToTiles;
		std::unordered_set<id> mAllTiles;
	};
}