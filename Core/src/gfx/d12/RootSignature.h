#pragma once
#include <vector>

#include "cmn/D12Headers.h"
#include "DeviceD12.h"

namespace CPR::GFX::D12
{
	struct RenderPassInfo;

	class RootSignature
	{
	public:
		RootSignature(IDevice* device);
		~RootSignature();
		operator ID3D12RootSignature* () const { return _rootSignature.Get(); }
	private:
		D3D12_ROOT_PARAMETER CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE typeOfView,
			D3D12_SHADER_VISIBILITY visibleShader, u32 shaderRegister, u32 registerSpace = 0);
		D3D12_DESCRIPTOR_RANGE CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type,
			u32 nrOfDescriptors, u32 baseShaderRegister, u32 registerSpace = 0);
		D3D12_ROOT_PARAMETER CreateDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>&, D3D12_SHADER_VISIBILITY);
		D3D12_ROOT_PARAMETER CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE&, D3D12_SHADER_VISIBILITY);
		D3D12_STATIC_SAMPLER_DESC CreateStaticSampler(D3D12_FILTER filter, UINT shaderRegister,
			D3D12_SHADER_VISIBILITY visibleShader = D3D12_SHADER_VISIBILITY_PIXEL, UINT registerSpace = 0);
	private:
		ComPtr<ID3D12RootSignature> _rootSignature;
	};
}
