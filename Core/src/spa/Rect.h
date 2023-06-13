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
				.left = topLeft.x,
				.top = topLeft.y,
				.right = topLeft.x + dimensions.width,
				.bottom = topLeft.y - dimensions.height,
			};
		}
		static RectT FromPoints(const Vec2T<T>& topLeft, const Vec2T<T>& bottomRight)
		{
			return {
				.left = topLeft.x,
				.top = topLeft.y,
				.right = bottomRight.x,
				.bottom = bottomRight.y,
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
		Vec2T<T> GetTopLeft() const { return {top, left}; }
		Vec2T<T> GetBottomLeft() const { return {bottom, left}; }
		Vec2T<T> GetTopRight() const { return {top, right}; }
		Vec2T<T> GetBottomRight() const { return {bottom, right}; }
		DimensionsT<T> GetDimensions() const { return { right - left, top - bottom}; }
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
