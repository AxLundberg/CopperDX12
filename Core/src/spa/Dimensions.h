#pragma once

namespace CPR::SPA
{
	template <typename T>
	struct DimensionsT
	{
		// data
		T width, height;
		// functions
		T GetArea()
		{
			return width * height;
		}
		// operators
		bool operator==(const DimensionsT& rhs) const
		{
			return width == rhs.width && height == rhs.width;
		}
	};
	using DimensionsF = DimensionsT<float>;
	using DimensionsI = DimensionsT<int>;
}
