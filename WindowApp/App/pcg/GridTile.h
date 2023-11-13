#pragma once
#include <Core/src/spa/Vec2.h>
#include <Core/src/spa/Rect.h>
#include "../TypeDefs.h"
#include <unordered_map>
#include <vector>

namespace CPR::APP
{
	using id = i32;
	using section = std::pair<u32, f32>;
	using location = std::pair<u32, u32>;
	using point = std::pair<f32, f32>;

	static constexpr i32 NORTH = 0;
	static constexpr i32 EAST = 1;
	static constexpr i32 SOUTH = 2;
	static constexpr i32 WEST = 3;
	static constexpr i32 DIRECTION[] = { NORTH, EAST, SOUTH, WEST };

	static constexpr u32 REFLECTX = 1;
	static constexpr i32 FREE = -1;

	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator () (const std::pair<T1, T2>& p) const {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);

			return h1 ^ h2;
		}
	};

	enum class GeometryType
	{
		Free,
		Triangle,
		Rectangle,
		Line
	};

	struct Geometry
	{
		GeometryType type = GeometryType::Free;
		u32 materialId = 0;
		CPR::SPA::Vec2F p1{};
		CPR::SPA::Vec2F p2{};
		CPR::SPA::Vec2F p3{};
	};
	struct Side
	{
		std::vector<std::pair<u32, u32>> edgeSections;

		bool operator ==(const Side& other) const
		{
			return edgeSections == other.edgeSections;
		}
	};
	struct TileSide
	{
		std::vector<section> edgeSections;

		bool operator ==(const TileSide& other) const
		{
			static constexpr f32 epsilon = 0.0001f;
			if (edgeSections.size() != other.edgeSections.size())
				return false;
			for (u32 i = 0; i < edgeSections.size(); ++i)
				if (edgeSections[i].first != other.edgeSections[i].first ||
					std::abs(edgeSections[i].second - other.edgeSections[i].second) > epsilon)
					return false;
			return true;
		}
	};
	struct AdjacencyHandle
	{
		i32 id = -1;
		bool reflectX = false;
	};
	struct GridTile
	{
		i32 invalidRotation = -1;
		i32 sideIDs[4] = { -1, -1, -1, -1 };
		std::vector<Geometry> geometry;
	};
	struct TileHandle {
		i32 id = -1;
		i32 rotation = 0;
		i32 reflect = 0;

		bool operator<(const TileHandle& other) const {
			return std::tie(id, rotation, reflect) < std::tie(other.id, other.rotation, other.reflect);
		}
		bool operator==(const TileHandle& other) const {
			return id == other.id &&
				rotation == other.rotation &&
				reflect == other.reflect;
		}
	};
}