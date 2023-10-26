#include "RootSignature.h"

namespace CPR::GFX::D12
{
	RootSignature::RootSignature(IDevice* device)
	{
		std::vector<D3D12_ROOT_PARAMETER> rootParameters;
		u32 vsCbvReg = 0, vsSrvReg = 0;
		rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, vsCbvReg++));
		rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, vsCbvReg++));
		rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX, vsSrvReg++));
		rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX, vsSrvReg++));

		u32 psCbvReg = 0, psSrvReg = 0;
		std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
		ranges.push_back(CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, psSrvReg++)); // diffuse
		ranges.push_back(CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, psSrvReg++)); // specular
		ranges.push_back(CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, psSrvReg++)); // light
		rootParameters.push_back(CreateDescriptorTable(ranges[0], D3D12_SHADER_VISIBILITY_PIXEL)); // diffuse
		rootParameters.push_back(CreateDescriptorTable(ranges[1], D3D12_SHADER_VISIBILITY_PIXEL)); // specular
		rootParameters.push_back(CreateDescriptorTable(ranges[2], D3D12_SHADER_VISIBILITY_PIXEL)); // light
		rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, psCbvReg++));

		auto samplerDesc = CreateStaticSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, 0u);

		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = static_cast<u32>(rootParameters.size());
		desc.pParameters = rootParameters.size() == 0 ? nullptr : rootParameters.data();
		desc.NumStaticSamplers = 1u;
		desc.pStaticSamplers = &samplerDesc;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

		ID3DBlob* rootSignatureBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &errorBlob) >> hrVerify;

		device->AsD3D12Device()->CreateRootSignature(0u, rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)) >> hrVerify;

		rootSignatureBlob->Release();
	}
	D3D12_ROOT_PARAMETER RootSignature::CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE typeOfView,
		D3D12_SHADER_VISIBILITY visibleShader, u32 shaderRegister, u32 registerSpace)
	{
		D3D12_ROOT_PARAMETER toReturn = {};
		toReturn.ParameterType = typeOfView;
		toReturn.ShaderVisibility = visibleShader;
		toReturn.Descriptor.ShaderRegister = shaderRegister;
		toReturn.Descriptor.RegisterSpace = registerSpace;
		return toReturn;
	}
	D3D12_DESCRIPTOR_RANGE RootSignature::CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type,
		u32 nrOfDescriptors, u32 baseShaderRegister, u32 registerSpace)
	{
		D3D12_DESCRIPTOR_RANGE toReturn = {};
		toReturn.RangeType = type;
		toReturn.NumDescriptors = nrOfDescriptors;
		toReturn.BaseShaderRegister = baseShaderRegister;
		toReturn.RegisterSpace = registerSpace;
		toReturn.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		return toReturn;
	}
	D3D12_ROOT_PARAMETER RootSignature::CreateDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges, D3D12_SHADER_VISIBILITY visibleShader)
	{
		// Do not use initializer list temporary vectors as input to this function
		// Memory needs to be valid after returning
		D3D12_ROOT_PARAMETER toReturn = {};
		toReturn.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		toReturn.ShaderVisibility = visibleShader;
		toReturn.DescriptorTable.NumDescriptorRanges = static_cast<u32>(ranges.size());
		toReturn.DescriptorTable.pDescriptorRanges = ranges.size() != 0 ? ranges.data() : nullptr;
		return toReturn;
	}
	D3D12_ROOT_PARAMETER RootSignature::CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE& range, D3D12_SHADER_VISIBILITY visibleShader)
	{
		// Do not use initializer list temporary vectors as input to this function
		// Memory needs to be valid after returning
		D3D12_ROOT_PARAMETER toReturn = {};
		toReturn.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		toReturn.ShaderVisibility = visibleShader;
		toReturn.DescriptorTable.NumDescriptorRanges = 1u;
		toReturn.DescriptorTable.pDescriptorRanges = &range;
		return toReturn;
	}
	D3D12_STATIC_SAMPLER_DESC RootSignature::CreateStaticSampler(D3D12_FILTER filter, UINT shaderRegister,
		D3D12_SHADER_VISIBILITY visibleShader, UINT registerSpace)
	{
		D3D12_STATIC_SAMPLER_DESC toReturn = {};
		toReturn.Filter = filter;
		toReturn.AddressU = toReturn.AddressV = toReturn.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		toReturn.MipLODBias = 0.0f;
		toReturn.MaxAnisotropy = 16;
		toReturn.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		toReturn.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		toReturn.MinLOD = 0;
		toReturn.MaxLOD = D3D12_FLOAT32_MAX;
		toReturn.ShaderRegister = shaderRegister;
		toReturn.RegisterSpace = registerSpace;
		toReturn.ShaderVisibility = visibleShader;
		return toReturn;
	}
	RootSignature::~RootSignature()
	{}
}
