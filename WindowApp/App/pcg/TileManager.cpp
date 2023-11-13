#include "TileManager.h"
#include <iterator>
#include <ranges>
#include <random>
#include <filesystem>

namespace CPR::APP
{
	namespace rn = std::ranges;
	namespace vi = rn::views;

	TileManager::TileManager()
	{
	}
	TileManager::~TileManager()
	{
	}
	std::vector<Tile> TileManager::CreateTiles(std::string pathToImages)
	{
		i32 fileCount = 0;

		for (const auto& entry : std::filesystem::directory_iterator(pathToImages))
		{
			if (entry.is_regular_file())
			{
				std::string filename = entry.path().filename().string();
				// Check if filename matches pattern t#.jpg
				if (filename.size() >= 6 &&
					filename.substr(0, 1) == "t" &&
					std::all_of(filename.begin() + 1, filename.begin() + filename.size() - 4, ::isdigit) &&
					filename.substr(filename.size() - 4) == ".jpg")
				{
					fileCount++;
				}
			}
		}
		std::vector<Tile> toReturn;
		for (i32 i = 0; i < fileCount; i++) {
			std::wstring filename = L"t" + std::to_wstring(i) + L".jpg";
			toReturn.push_back(Tile{ filename });
		}
		return toReturn;
	}
	Material CreateMaterial(u8 r, u8 g, u8 b, u8 a)
	{
		static const f32 stepSize = std::round(255.0f / COLOR_GRANULARITY);
		u8 red = static_cast<u8>(std::round(r / stepSize) * stepSize);
		u8 green = static_cast<u8>(std::round(g / stepSize) * stepSize);
		u8 blue = static_cast<u8>(std::round(b / stepSize) * stepSize);
		return Material{ red, green, blue, a };
	}
	std::vector<Material> TileManager::CreateMaterials()
	{
		// Lambda function to calculate color
		auto calcColor = [](u32 val) {
			static const f32 stepSize = std::round(255.0f / COLOR_GRANULARITY);
			f32 color = val * stepSize;
			color = color > 255.f ? 255.f : color;
			return static_cast<u8>(color);
		};

		std::vector<Material> toReturn;
		for (u32 b = 0; b <= COLOR_GRANULARITY; b++)
		{
			for (u32 g = 0; g <= COLOR_GRANULARITY; g++)
			{
				for (u32 r = 0; r <= COLOR_GRANULARITY; r++)
				{
					toReturn.push_back(Material{
						.r = calcColor(r),
						.g = calcColor(g),
						.b = calcColor(b),
						.a = 255
						});
				}
			}
		}
		return toReturn;
	}

	std::unordered_map<id, std::unordered_set<id>> TileManager::CreateAdjacencyMap(std::vector<Tile>& tiles)
	{
		std::unordered_map<id, std::unordered_set<id>> adjacencyMap;

		std::unordered_map<id, Side> sideMap = CreateSides(tiles);
		for (auto& ts : sideMap)
		{
			for (u32 j = 0; j < tiles.size(); j++)
			{
				if (IsCompatible(ts.first, tiles[j]))
					adjacencyMap[ts.first].insert(j);
			}
		}

		return adjacencyMap;
	}

	bool TileManager::IsCompatible(id sideID, Tile& tile)
	{
		auto& tileSides = tile.sideIDs;
		for (i32 tsID : tileSides)
		{
			if (tsID == sideID || tsID == -sideID)
			{
				return true;
			}
		}
		return false;
	}

	std::unordered_map<id, Side> TileManager::CreateSides(std::vector<Tile>& tiles)
	{
		std::unordered_map<id, Side> toReturn;
		for (auto& tile : tiles)
		{
			Side side[4];
			side[NORTH].edgeSections = tile.GetEdge(NORTH);
			side[EAST].edgeSections = tile.GetEdge(EAST);
			side[SOUTH].edgeSections = tile.GetEdge(SOUTH);
			side[WEST].edgeSections = tile.GetEdge(WEST);
			tile.sideIDs[NORTH] = SetTileSideID(toReturn, side[NORTH]);
			tile.sideIDs[EAST] = SetTileSideID(toReturn, side[EAST]);
			tile.sideIDs[SOUTH] = SetTileSideID(toReturn, side[SOUTH]);
			tile.sideIDs[WEST] = SetTileSideID(toReturn, side[WEST]);
		}

		return toReturn;
	}

	id TileManager::SetTileSideID(std::unordered_map<id, Side>& sideMap, Side tileSide)
	{
		for (auto& e : tileSide.edgeSections)
		{
			for (auto& ts : sideMap)
			{ // check if tile side already exists, if so return its id
				auto [sideID, ots] = ts;
				if (tileSide == ots) {
					return sideID;
				}
			}
		}
		// edgeSection is new and needs to be stored
		i32 tsID = static_cast<i32>(sideMap.size());
		if (bool hasReflection = tileSide.edgeSections.size() > 1; hasReflection)
		{
			Side reflected;
			for (auto& e : vi::reverse(tileSide.edgeSections)) {
				reflected.edgeSections.push_back(e);
			}
			sideMap[-tsID] = reflected;
		}
		// finally, store tile side, and return its id
		sideMap[tsID] = tileSide;
		return tsID;
	}
}