#pragma once
#include <vector>
#include <d3d11_4.h>

#include "cmn/D11Headers.h"

namespace CPR::GFX::D11
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
	class SamplerManagerD11
	{
	public:
		SamplerManagerD11();
		~SamplerManagerD11();
		void Initialise(ComPtr<ID3D11Device> deviceToUse);

		ResourceIndex CreateSampler(SamplerType type, AddressMode adressMode);

		ID3D11SamplerState* GetSampler(ResourceIndex index);

	private:
		void SetFilter(D3D11_SAMPLER_DESC& toSetIn, SamplerType type);
		void SetAdressMode(D3D11_SAMPLER_DESC& toSetIn, AddressMode adressMode);

		ComPtr<ID3D11Device> device = nullptr;
		std::vector<ID3D11SamplerState*> samplers;
	};
}
