#include <fstream>
#include <stdexcept>
#include <d3dcompiler.h>

#include "RenderPassD11.h"

namespace CPR::GFX::D11
{
	GfxRenderPassD11::GfxRenderPassD11(ComPtr<ID3D11Device> device, const RenderPassInfo& info)
		:
		device(device),
		objectBindings(info.objectBindings),
		globalBindings(info.globalBindings)
	{
		ComPtr<ID3DBlob> vsBlob = LoadCSO(info.vsPath);
		device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			nullptr, vertexShader.GetAddressOf());
		ComPtr<ID3DBlob> psBlob = LoadCSO(info.psPath);
		device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
			nullptr, pixelShader.GetAddressOf());
	}

	GfxRenderPassD11::~GfxRenderPassD11()
	{
	}

	ComPtr<ID3DBlob> GfxRenderPassD11::LoadCSO(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Could not open CSO file");

		file.seekg(0, std::ios_base::end);
		size_t size = static_cast<size_t>(file.tellg());
		file.seekg(0, std::ios_base::beg);

		ComPtr<ID3DBlob> toReturn;
		HRESULT hr = D3DCreateBlob(size, &toReturn);

		if (FAILED(hr))
			throw std::runtime_error("Could not create blob when loading CSO");

		file.read(static_cast<char*>(toReturn->GetBufferPointer()), size);
		file.close();

		return toReturn;
	}

	void GfxRenderPassD11::SetShaders(ID3D11DeviceContext* deviceContext)
	{
		deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
		deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	const std::vector<PipelineBinding>& GfxRenderPassD11::GetObjectBindings()
	{
		return objectBindings;
	}

	const std::vector<PipelineBinding>& GfxRenderPassD11::GetGlobalBindings()
	{
		return globalBindings;
	}

	void GfxRenderPassD11::SetGlobalSampler(PipelineShaderStage shader,
		std::uint8_t slot, ResourceIndex index)
	{
		switch (shader)
		{
		case PipelineShaderStage::VS:
			vsGlobalSamplers[slot] = index;
			break;
		case PipelineShaderStage::PS:
			psGlobalSamplers[slot] = index;
			break;
		default:
			throw std::runtime_error("Incorrect shader stage when setting sampler");
		}
	}

	ResourceIndex GfxRenderPassD11::GetGlobalSampler(PipelineShaderStage shader,
		std::uint8_t slot) const
	{
		switch (shader)
		{
		case PipelineShaderStage::VS:
			return vsGlobalSamplers[slot];
			break;
		case PipelineShaderStage::PS:
			return psGlobalSamplers[slot];
			break;
		default:
			throw std::runtime_error("Incorrect shader stage when fetching sampler");
		}
	}
}
