#pragma once
#include "GridTile.h"
#include "Tile.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>

namespace CPR::APP
{
	class TileManager
	{
	public:
		TileManager();
		~TileManager();
		std::vector<Tile> CreateTiles(std::string pathToImages);
		std::vector<Material> CreateMaterials();
		std::unordered_map<id, std::unordered_set<id>> CreateAdjacencyMap(std::vector<Tile>&);
	private:
		bool IsCompatible(id sideID, Tile& tile);
		std::unordered_map<id, Side> CreateSides(std::vector<Tile>&);
		id SetTileSideID(std::unordered_map<id, Side>& sideMap, Side tileSide);

	};
}