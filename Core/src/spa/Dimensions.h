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
		DimensionsT operator/(T divisor) const
		{
			return { width / divisor, height / divisor };
		}
		template<typename S>
		operator DimensionsT<S>() const
		{
			return { (S)width, (S)height };
		}
		DimensionsT operator*(const DimensionsT& rhs) const
		{
			return { width * rhs.width, height * rhs.height };
		}
	};
	using DimensionsF = DimensionsT<float>;
	using DimensionsI = DimensionsT<int>;
}
