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
	};
	using DimensionsF = DimensionsT<float>;
	using DimensionsI = DimensionsT<int>;
}
