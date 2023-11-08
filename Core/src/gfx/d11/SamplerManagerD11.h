#pragma once
#include <vector>
#include <memory>
#include <d3d11_4.h>

#include "DeviceD11.h"
#include "../ISamplerManager.h"
#include "../cmn/TypeDefs.h"

namespace CPR::GFX::D11
{
	class ISamplerManager : public GFX::ISamplerManager
	{
	public:
		virtual ResourceIndex CreateSampler(SamplerType type, AddressMode adressMode) = 0;
		virtual ID3D11SamplerState* GetSampler(ResourceIndex index) = 0;
	};

	class SamplerManagerD11 : public ISamplerManager
	{
	public:
		SamplerManagerD11(std::shared_ptr<IDevice>);
		~SamplerManagerD11();

		ResourceIndex CreateSampler(SamplerType type, AddressMode adressMode) override;
		ID3D11SamplerState* GetSampler(ResourceIndex index) override;

	private:
		void SetFilter(D3D11_SAMPLER_DESC& toSetIn, SamplerType type);
		void SetAdressMode(D3D11_SAMPLER_DESC& toSetIn, AddressMode adressMode);

		std::shared_ptr<IDevice> device;
		std::vector<ID3D11SamplerState*> samplers;
	};
}
