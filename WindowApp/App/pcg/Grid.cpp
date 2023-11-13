#include "Grid.h"
#include <algorithm>
#include <iterator>

namespace CPR::APP
{
	Grid::Grid(u32 dim)
		: dim(dim), mStartPos({ 0,0 })
	{
		mTileHandles.resize(dim * dim);
		std::fill(mTileHandles.begin(), mTileHandles.end(), TileHandle{});
		mFrontier.push_back(mStartPos);
		mTiles = mTileManager.CreateTiles(PATH);
		mSideToTiles = mTileManager.CreateAdjacencyMap(mTiles);

		for (const auto& pair : mSideToTiles) {
			mAllTiles.insert(pair.second.begin(), pair.second.end());
			mSideToTiles[RED_TILE_ID].insert(pair.second.begin(), pair.second.end());
		}
		mAllTiles.erase(0);

		for (u32 i = 0; i < dim; i++)
			for (u32 j = 0; j < dim; j++)
				mLocToSuperPositions[{i, j}] = { ANY_TILE };
	}

	Grid::~Grid()
	{
	}

	void Grid::PlaceTile(location loc, TileHandle tileHandle)
	{
		auto [x, y] = loc;
		auto th = At(loc);
		th->id = tileHandle.id;
		th->reflect = tileHandle.reflect;
		th->rotation = tileHandle.rotation;
		RemoveFromFrontier(loc);
		AddNeighborsToFrontier(loc);
		UpdateFrontierNeighbors(loc);
	}
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
	void Grid::UpdateSuperPositions(location loc, i32 fromDir)
	{
		auto oppositeDir = OppositeDirection(fromDir);

		i32 sideBetween{};
		{
			auto adjacentLoc = LocAt(oppositeDir, loc);
			auto adjacentHandle = At(adjacentLoc.value());
			auto adjacentTile = mTiles[adjacentHandle->id];
			auto adjRot = adjacentHandle->rotation;
			auto adjSideIdx = AdjacentSideIndex(fromDir, adjRot);
			sideBetween = adjacentTile.sideIDs[adjSideIdx];
		}

		auto compatibleTiles = std::vector<TileHandle>();
		auto& tiles = mSideToTiles[sideBetween];
		for (auto tileID : tiles)
		{
			if (sideBetween == 0)
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
				i32 reqRot = (oppositeDir - dir) < 0 ? (oppositeDir - dir) + 4 : (oppositeDir - dir);
				compatibleTiles.push_back(TileHandle{
					.id = tileID,
					.rotation = reqRot,
					.reflect = 0
					});
			}
		}

		if (sideBetween == 0)
		{
			for (auto tileID : tiles)
			{
				auto& tile = mTiles[tileID];
				for (auto dir : DIRECTION)
				{
					compatibleTiles.push_back(TileHandle{
						.id = tileID,
						.rotation = dir,
						.reflect = 0
						});
				}
			}
		}

		if (mLocToSuperPositions[loc].empty())
			mLocToSuperPositions[loc].push_back(RED_TILE);
		else if (mLocToSuperPositions[loc][0] == ANY_TILE)
			mLocToSuperPositions[loc] = compatibleTiles;
		else
			mLocToSuperPositions[loc] = Intersect(mLocToSuperPositions[loc], compatibleTiles);

	}
	std::vector<TileHandle> Grid::Intersect(std::vector<TileHandle>& a, std::vector<TileHandle>& b) {
		// ensure the vectors are sorted
		std::sort(a.begin(), a.end());
		std::sort(b.begin(), b.end());

		// get the intersection
		std::vector<TileHandle> intersection;
		std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(intersection));

		return intersection;
	}

	void Grid::UpdateFrontierNeighbors(location loc)
	{
		for (auto dir : DIRECTION)
		{
			// check if valid pos at direction
			if (auto pos = LocAt(dir, loc); pos.has_value())
			{
				// make sure tile is already in frontier
				auto it = std::find_if(mFrontier.begin(), mFrontier.end(), [&](auto& p)
					{ return p == pos.value(); });
				if (it == mFrontier.end())
					continue;

				// if tile is in frontier, update the superpositions
				UpdateSuperPositions(pos.value(), dir);
			}
		}
	}

	void Grid::AddNeighborsToFrontier(location loc)
	{
		// add the adjacent tiles to the frontier
		for (auto dir : DIRECTION)
		{
			// check if valid pos at direction from the removed loc
			if (auto pos = LocAt(dir, loc); pos.has_value())
			{
				// check if tile is already in frontier
				auto it = std::find_if(mFrontier.begin(), mFrontier.end(), [&](auto& p)
					{ return p == pos.value(); });
				if (it != mFrontier.end())
					continue;
				// if not in frontier, check if the tile at that pos is empty
				if (auto tile = At(pos.value()); tile && tile->id == -1)
				{
					// if so, add to frontier
					mFrontier.push_back(pos.value());
					UpdateSuperPositions(pos.value(), dir);
				}
			}
		}
	}

	TileHandle* Grid::At(location loc)
	{
		auto [x, y] = loc;
		bool validIdx = x < dim && y < dim;
		return validIdx ? &mTileHandles[x + y * dim] : nullptr;
	}
	location Grid::GetStartPos()
	{
		return mStartPos;
	}

	void Grid::RemoveFromFrontier(location toRemove)
	{
		static u32 directions[] = { NORTH, EAST, SOUTH, WEST };
		// remove the newly placed location from the frontier
		auto it = std::find_if(mFrontier.begin(), mFrontier.end(), [&](auto& p) { return p == toRemove; });
		if (it != mFrontier.end())
			mFrontier.erase(it);
	}
	std::vector<location>& Grid::GetFrontier()
	{
		return mFrontier;
	}
	std::vector<TileHandle>& Grid::GetSuperPositions(location loc)
	{
		return mLocToSuperPositions[loc];
	}
	std::optional<location> Grid::LocAt(u32 dir, location loc) {
		auto [x, y] = loc;
		if (dir == NORTH || dir == SOUTH)
			y = dir == NORTH ? y + 1 : y - 1;
		if (dir == EAST || dir == WEST)
			x = dir == EAST ? x + 1 : x - 1;
		bool validIdx = x < dim && y < dim;
		return validIdx ? std::optional<location>({ x, y }) : std::nullopt;
	}

}