#pragma once
#include "Dimensions.h"
#include "Vec2.h"

namespace CPR::SPA
{
	template <typename T>
	struct RectT
	{
		static RectT FromTopLeftAndDimensions(const Vec2T<T>& topLeft, const DimensionsT<T>& dimensions)
		{
			return {
				.top = topLeft.y,
				.left = topLeft.x,
				.bottom = topLeft.y - dimensions.y,
				.right = topLeft.x + dimensions.x,
			};
		}
		static RectT FromPoints(const Vec2T<T>& topLeft, const Vec2T<T>& bottomRight)
		{
			return {
				.top = topLeft.y,
				.left = topLeft.x,
				.bottom = bottomRight.y,
				.right = bottomRight.x,
			};
		}
		static RectT FromMiddleAndDimensions(const Vec2T<T>& middle, const DimensionsT<T>& dimensions)
		{
			return {
				.top = middle.y + dimensions.height * 0.5f,
				.left = middle.x - dimensions.width * 0.5f,
				.bottom = middle.y - dimensions.height * 0.5f,
				.right = middle.x + dimensions.width * 0.5f,
			};
		}
		Vec2T GetTopLeft() const { return Vec2T<T>{top, left}; }
		Vec2T GetBottomLeft() const { return Vec2T<T>{bottom, left}; }
		Vec2T GetTopRight() const { return Vec2T<T>{top, right}; }
		Vec2T GetBottomRight() const { return Vec2T<T>{bottom, right}; }
		DimensionsT<T> GetDimensions() const { return DimensionsT<T>{ right - left, top - bottom}; }
		bool Contains(const RectT<T>& inside) const
		{
			return inside.top <= top && inside.bottom >= bottom && inside.left >= left && inside.right <= right;
		}

		// data
		T left, top, right, bottom;
		// operators
		
	};
	using RectF = RectT<float>;
	using RectI = RectT<int>;
}
