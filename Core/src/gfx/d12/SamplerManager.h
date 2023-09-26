#pragma once


namespace CPR::GFX::D12
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

	class SamplerManager
	{
	protected:

	public:
		SamplerManager() = default;
		virtual ~SamplerManager() = default;
		SamplerManager(const SamplerManager& other) = delete;
		SamplerManager& operator=(const SamplerManager& other) = delete;
		SamplerManager(SamplerManager&& other) = default;
		SamplerManager& operator=(SamplerManager&& other) = default;

		virtual int CreateSampler(SamplerType type, AddressMode adressMode) = 0;
	};
}