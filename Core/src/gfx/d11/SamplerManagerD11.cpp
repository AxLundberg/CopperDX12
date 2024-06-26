#include <stdexcept>

#include "../cmn/GraphicsError.h"
#include "SamplerManagerD11.h"

namespace CPR::GFX::D11
{
	SamplerManagerD11::SamplerManagerD11(std::shared_ptr<IDevice> device)
		:
		device(std::move(device))
	{}

	SamplerManagerD11::~SamplerManagerD11()
	{
		for (auto& sampler : samplers)
			sampler->Release();
	}

	void SamplerManagerD11::SetFilter(D3D11_SAMPLER_DESC& toSetIn,
		SamplerType type)
	{
		switch (type)
		{
		case SamplerType::POINT:
			toSetIn.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			toSetIn.MaxAnisotropy = 1;
			break;
		case SamplerType::BILINEAR:
			toSetIn.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			toSetIn.MaxAnisotropy = 1;
			break;
		case SamplerType::ANISOTROPIC:
			toSetIn.Filter = D3D11_FILTER_ANISOTROPIC;
			toSetIn.MaxAnisotropy = 16;
			break;
		default:
			break;
		}
	}

	void SamplerManagerD11::SetAdressMode(D3D11_SAMPLER_DESC& toSetIn,
		AddressMode adressMode)
	{
		switch (adressMode)
		{
		case AddressMode::WRAP:
			toSetIn.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			toSetIn.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			toSetIn.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case AddressMode::CLAMP:
			toSetIn.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			toSetIn.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			toSetIn.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case AddressMode::BLACK_BORDER_COLOUR:
			toSetIn.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
			toSetIn.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
			toSetIn.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
			break;
		default:
			throw std::runtime_error("Incorrect adress mode when creating sampler");
		}

		memset(toSetIn.BorderColor, 0, sizeof(float) * 4);
	}

	ResourceIndex SamplerManagerD11::CreateSampler(SamplerType type,
		AddressMode adressMode)
	{
		D3D11_SAMPLER_DESC desc;
		desc.MipLODBias = 0.0f;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0;
		desc.MaxLOD = FLT_MAX;

		SetFilter(desc, type);
		SetAdressMode(desc, adressMode);

		ID3D11SamplerState* toStore = nullptr;
		device->GetD3D11Device()->CreateSamplerState(&desc, &toStore) >> hrVerify;

		samplers.push_back(toStore);
		return static_cast<ResourceIndex>(samplers.size() - 1);
	}

	ID3D11SamplerState* SamplerManagerD11::GetSampler(ResourceIndex index)
	{
		return samplers[index];
	}
}
