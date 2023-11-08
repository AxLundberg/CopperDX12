#pragma once

namespace CPR::GFX
{
	enum class SamplerType
	{
		POINT,
		BILINEAR,
		ANISOTROPIC
	};

	enum class AddressMode
	{
		WRAP,
		CLAMP,
		BLACK_BORDER_COLOUR
	};

	class ISamplerManager
	{
	public:
		virtual ~ISamplerManager() = default;
	};
}
