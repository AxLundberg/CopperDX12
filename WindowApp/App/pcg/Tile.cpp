#include "Tile.h"
#include "GridTile.h"
#include <iostream>
#include <array>
#include <cmath>
#include <Core/src/log/Log.h>
#include <Core/src/utl/Assert.h>
#pragma warning (push)
#pragma warning (disable : 26451 26819 6262)
#include "../../3rd/stb_image.h"
#pragma warning (pop)

namespace CPR::APP
{
	Tile::Tile()
		: mTexelData(TILE_TEXEL_COUNT * TILE_TEXEL_COUNT, RED)
	{
	}
	Tile::Tile(std::wstring imagePath)
		: mTexelData(TILE_TEXEL_COUNT* TILE_TEXEL_COUNT, RED)
	{
		for (u32 i = 0; i < NR_OF_DIRECTIONS; i++)
			mEdges.push_back({});

		FillEdges(imagePath);

		i32 stbWidth, stbHeight, channels;
		unsigned char* stbImage = stbi_load(CPR::UTL::ToNarrow(imagePath).c_str(), &stbWidth, &stbHeight, &channels, 0);
		if (stbImage == NULL) {
			std::cout << "Could not read the image file" << std::endl;
			return;
		}
		name = imagePath.substr(imagePath.size() - 5, 3);
		i32 scale = stbWidth / TILE_TEXEL_COUNT;

		for (i32 i = 0; i < TILE_TEXEL_COUNT; ++i) {
			for (i32 j = 0; j < TILE_TEXEL_COUNT; ++j) {
				u32 average[3] = { 0, 0, 0 }; // Average color for each channel

				// Loop over corresponding area in source image
				for (i32 y = 0; y < scale; ++y) {
					for (i32 x = 0; x < scale; ++x) {
						i32 stbX = j * scale + x;
						i32 stbY = i * scale + y;

						// Add pixel color to sum
						for (i32 c = 0; c < 3; ++c) {
							average[c] += stbImage[(stbY * stbWidth + stbX) * channels + c];
						}
					}
				}
				u32 r = average[0], g = average[1], b = average[2];
				// Divide by total number of pixels in area to get average color
				r /= scale * scale;
				g /= scale * scale;
				b /= scale * scale;

				u32 pixelColor = (r << 16) | (g << 8) | b;

				// Assign average color to destination pixel
				auto apxColor = ApproximateColor(pixelColor);
				mTexelData[i * TILE_TEXEL_COUNT + j] = apxColor;
			}
		}
	}
	Tile::~Tile()
	{}
	col Tile::ApproximateColor(col color)
	{
		static constexpr double step = 255.0 / COLOR_GRANULARITY;

		u32 r1 = (color >> 16) & 0xFF;
		u32 g1 = (color >> 8) & 0xFF;
		u32 b1 = color & 0xFF;

		r1 = static_cast<u32>(std::round(r1 / step) * step);
		g1 = static_cast<u32>(std::round(g1 / step) * step);
		b1 = static_cast<u32>(std::round(b1 / step) * step);
		return (r1 << 16) | (g1 << 8) | b1;
	}
	void Tile::ToImage(std::wstring imageName)
	{
	}
	std::vector<std::pair<u32, col>> Tile::GetEdge(u32 side)
	{
		i32 start{}, step{};
		// deterime start, end, and step based on tile side
		switch (side)
		{
		case NORTH:
			start = 0;
			step = 1;
			break;
		case EAST:
			start = TILE_TEXEL_COUNT - 1;
			step = TILE_TEXEL_COUNT;
			break;
		case SOUTH:
			start = TILE_TEXEL_COUNT * TILE_TEXEL_COUNT - 1;
			step = -1;
			break;
		case WEST:
			start = TILE_TEXEL_COUNT * TILE_TEXEL_COUNT - TILE_TEXEL_COUNT;
			step = -TILE_TEXEL_COUNT;
			break;
		}

		
		std::vector<std::pair<u32, col>> result;
		{
			u32 count = 0;
			col color = mTexelData[start];
			for (i32 i = 0; i < TILE_TEXEL_COUNT; ++i)
			{
				auto idx = start + i * step;
				if (mTexelData[idx] == color)
				{
					++count;
				}
				else
				{
					result.push_back({ count, color });
					count = 1;
					color = mTexelData[idx];
				}
			}
			result.push_back({ count, color });
		}

		std::vector<std::pair<u32, col>> res2;
		{
			u32 count = 0;
			col color = mEdges[side][0];
			for (i32 i = 0; i < TILE_TEXEL_COUNT; ++i)
			{
				auto idx = i;
				if (mEdges[side][idx] == color)
				{
					++count;
				}
				else
				{
					res2.push_back({ count, color });
					count = 1;
					color = mEdges[side][idx];
				}
			}
			res2.push_back({ count, color });
		}

		/*if (result.size() != res2.size())
		{
			auto asd = 0;
		}
		for (i32 i = 0; i < result.size(); i++)
		{
			if (result[i] != res2[i])
			{
				auto tst = 9;
			}
		}*/
		auto nafs = name;
		auto asdasd = -0;
		return res2;
	}
	u32 Tile::ColorToMaterial(col color)
	{
		u8 red = (color >> 16) & 0xFF;
		u8 green = (color >> 8) & 0xFF;
		u8 blue = color & 0xFF;
		u32 toRet = static_cast<u32>(std::round(red / 255.f * COLOR_GRANULARITY)) +
			static_cast<u32>(std::round(green / 255.f * COLOR_GRANULARITY)) * COLOR_GRANULARITY +
			static_cast<u32>(std::round(blue / 255.f * COLOR_GRANULARITY)) * COLOR_GRANULARITY * COLOR_GRANULARITY;
		return toRet;
	}
	u32 Tile::ColorToMaterial(u8 r, u8 g, u8 b)
	{
		u32 toRet = static_cast<u32>(std::round(r / 255.f * COLOR_GRANULARITY)) +
			static_cast<u32>(std::round(g / 255.f * COLOR_GRANULARITY)) * COLOR_GRANULARITY +
			static_cast<u32>(std::round(b / 255.f * COLOR_GRANULARITY)) * COLOR_GRANULARITY * COLOR_GRANULARITY;
		return toRet;
	}
	i32 Tile::GetSideID(i32 fromDir, i32 rotation)
	{
		auto oppositeDir = (fromDir + NR_OF_DIRECTIONS / 2) % NR_OF_DIRECTIONS;
		auto sideIndex = (NR_OF_DIRECTIONS + oppositeDir - rotation) % NR_OF_DIRECTIONS;
		return sideIDs[sideIndex];
	}
	void Tile::FillEdges(std::wstring imagePath)
	{
		i32 stbWidth, stbHeight, channels;
		unsigned char* stbImage = stbi_load(CPR::UTL::ToNarrow(imagePath).c_str(), &stbWidth, &stbHeight, &channels, 0);
		if (stbImage == NULL) {
			std::cout << "Could not read the image file" << std::endl;
			return;
		}

		name = imagePath.substr(imagePath.size() - 5, 3);
		
		cpr_assert(stbWidth == stbHeight);

		i32 scale = stbWidth / TILE_TEXEL_COUNT;
		for (auto dir : DIRECTION)
		{
			i32 start{}, step{};
			switch (dir)
			{
			case NORTH:
				start = 0;
				step = 1;
				break;
			case EAST:
				start = stbWidth - 1;
				step = stbWidth;
				break;
			case SOUTH:
				start = stbWidth * stbWidth - 1;
				step = -1;
				break;
			case WEST:
				start = stbWidth * stbWidth - stbWidth;
				step = -stbWidth;
				break;
			}

			for (i32 i = 0; i < TILE_TEXEL_COUNT; i++)
			{
				u32 average[3] = { 0, 0, 0 }; // Average color for each channel
				for (i32 j = 0; j < scale; j++)
				{
					i32 stbPixIdx = start + i * scale * step + j * step;

					average[0] += stbImage[stbPixIdx * channels + 0];
					average[1] += stbImage[stbPixIdx * channels + 1];
					average[2] += stbImage[stbPixIdx * channels + 2];
				}
				u32 r = average[0], g = average[1], b = average[2];
				// Divide by total number of pixels in area to get average color
				r /= scale;
				g /= scale;
				b /= scale;

				u32 pixelColor = (r << 16) | (g << 8) | b;

				// Assign average color to destination pixel
				auto apxColor = ApproximateColor(pixelColor);
				mEdges[dir].push_back(apxColor);
			}
		}
	}
}
