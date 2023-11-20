#pragma once
#include <vector>
#include <DirectXMath.h>
#include "tileData.h"

namespace CPR::APP
{
	class Tile
	{
	public:
		std::wstring name;
		i32 sideIDs[4] = { -1, -1, -1, -1 };
		u32 renderObjectIndex;
		DirectX::XMFLOAT4X4 transformation;
		u32 tileTexture;
		u32 x;
		u32 y;
	public:
		Tile();
		Tile(std::wstring imagePath);
		~Tile();
		col ApproximateColor(col color);
		void ToImage(std::wstring imageName);
		std::vector<u32>& GetTexels() { return mTexelData; };
		std::vector<std::pair<u32, col>> GetEdge(u32 dir);
		i32 GetSideID(i32 fromDir, i32 rotation);
	private:
		u32 ColorToMaterial(col color);
		u32 ColorToMaterial(u8 r, u8 g, u8 b);
	private:
		std::vector<u32> mTexelData;
	};
}