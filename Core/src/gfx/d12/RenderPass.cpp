#include <fstream>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "RenderPass.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	GfxRenderPass::GfxRenderPass(IDevice* device, const RenderPassInfo& info)
		: 
		_rootSignature(device, info),
		_pipelineState(device, info, _rootSignature),
		_objectBindings(info.objectBindings),
		_globalBindings(info.globalBindings)
	{}
	void GfxRenderPass::SetGlobalSampler(PipelineShaderStage shader, u8 slot, ResourceIndex index)
	{
		switch (shader)
		{
		case PipelineShaderStage::VS:
			_vsGlobalSamplers[slot] = index;
			break;
		case PipelineShaderStage::PS:
			_psGlobalSamplers[slot] = index;
			break;
		default:
			throw std::runtime_error("Incorrect shader stage when setting sampler");
		}
	}
	ResourceIndex GfxRenderPass::GetGlobalSampler(PipelineShaderStage shader, u8 slot) const
	{
		switch (shader)
		{
		case PipelineShaderStage::VS:
			return _vsGlobalSamplers[slot];
			break;
		case PipelineShaderStage::PS:
			return _psGlobalSamplers[slot];
			break;
		default:
			throw std::runtime_error("Incorrect shader stage when fetching sampler");
		}
	}
	RootSignature::RootSignature(IDevice* device, const RenderPassInfo& info)
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
	//////////////////////////////////////
	PipelineState::PipelineState(IDevice* d, const RenderPassInfo& info, ID3D12RootSignature* rootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
		ZeroMemory(&desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		desc.pRootSignature = rootSignature;
		std::array<D3D12_SHADER_BYTECODE*, 5> shaders = { &desc.VS, &desc.HS, &desc.DS, &desc.GS, &desc.PS };
		std::array<ID3DBlob*, 5> shaderBlobs;

		const std::array<std::string, 5> shaderPaths = { info.vsPath, "", "", "", info.psPath }; // AXEL
		const DXGI_FORMAT rtvFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM }; // AXEL
		const DXGI_FORMAT dsvFormat = DXGI_FORMAT_D32_FLOAT; // AXEL

		for (int i = 0; i < 5; ++i)
		{
			if (shaderPaths[i] != "")
			{
				shaderBlobs[i] = LoadCSO(shaderPaths[i]);
				(*shaders[i]).pShaderBytecode = shaderBlobs[i]->GetBufferPointer();
				(*shaders[i]).BytecodeLength = shaderBlobs[i]->GetBufferSize();
			}
		}

		desc.SampleMask = u32(-1);
		desc.RasterizerState = CreateRasterizerDesc();
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = ARRAYSIZE(rtvFormats);

		desc.BlendState.AlphaToCoverageEnable = false;
		desc.BlendState.IndependentBlendEnable = false;

		for (unsigned int i = 0; i < ARRAYSIZE(rtvFormats); ++i)
		{
			desc.RTVFormats[i] = rtvFormats[i];
			desc.BlendState.RenderTarget[i] = CreateBlendDesc();
		}

		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.DSVFormat = dsvFormat;
		desc.DepthStencilState = CreateDepthStencilDesc();
		desc.StreamOutput = CreateStreamOutputDesc();
		desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

		auto device = d->AsD3D12Device();
		device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&_pipelineState)) >> hrVerify;
	}
	ID3DBlob* PipelineState::LoadCSO(const std::string& filepath)
	{
		std::ifstream file(filepath, std::ios::binary);

		if (!file.is_open())
			throw std::runtime_error("Failed to open CSO file");

		file.seekg(0, std::ios_base::end);
		u64 size = static_cast<u64>(file.tellg());
		file.seekg(0, std::ios_base::beg);

		ID3DBlob* toReturn = nullptr;
		D3DCreateBlob(size, &toReturn) >> hrVerify;

		file.read(static_cast<char*>(toReturn->GetBufferPointer()), size);
		file.close();

		return toReturn;
	}
	D3D12_RASTERIZER_DESC PipelineState::CreateRasterizerDesc()
	{
		D3D12_RASTERIZER_DESC toReturn{};
		toReturn.FillMode = D3D12_FILL_MODE_SOLID;
		toReturn.CullMode = D3D12_CULL_MODE_BACK;
		toReturn.FrontCounterClockwise = false;
		toReturn.DepthBias = 0;
		toReturn.DepthBiasClamp = 0.0f;
		toReturn.SlopeScaledDepthBias = 0.0f;
		toReturn.DepthClipEnable = true;
		toReturn.MultisampleEnable = false;
		toReturn.AntialiasedLineEnable = false;
		toReturn.ForcedSampleCount = 0;
		toReturn.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		return toReturn;
	}
	D3D12_RENDER_TARGET_BLEND_DESC PipelineState::CreateBlendDesc()
	{
		D3D12_RENDER_TARGET_BLEND_DESC toReturn{};
		toReturn.BlendEnable = false;
		toReturn.LogicOpEnable = false;
		toReturn.SrcBlend = D3D12_BLEND_ONE;
		toReturn.DestBlend = D3D12_BLEND_ZERO;
		toReturn.BlendOp = D3D12_BLEND_OP_ADD;
		toReturn.SrcBlendAlpha = D3D12_BLEND_ONE;
		toReturn.DestBlendAlpha = D3D12_BLEND_ZERO;
		toReturn.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		toReturn.LogicOp = D3D12_LOGIC_OP_NOOP;
		toReturn.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		return toReturn;
	}
	D3D12_DEPTH_STENCIL_DESC PipelineState::CreateDepthStencilDesc()
	{
		D3D12_DEPTH_STENCIL_DESC toReturn{};
		toReturn.DepthEnable = true;
		toReturn.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		toReturn.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		toReturn.StencilEnable = false;
		toReturn.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		toReturn.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		toReturn.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		toReturn.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		toReturn.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		toReturn.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		toReturn.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		toReturn.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		toReturn.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		toReturn.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		return toReturn;
	}
	D3D12_STREAM_OUTPUT_DESC PipelineState::CreateStreamOutputDesc()
	{
		D3D12_STREAM_OUTPUT_DESC toReturn = {};
		toReturn.pSODeclaration = nullptr;
		toReturn.NumEntries = 0;
		toReturn.pBufferStrides = nullptr;
		toReturn.NumStrides = 0;
		toReturn.RasterizedStream = 0;
		return toReturn;
	}
	PipelineState::~PipelineState()
	{}
	/////////////////////////////////
	//RenderPass::RenderPass(ComPtr<ID3D12Device5> const& device, const RenderPassInfo& info)
	//	:
	//	_device(device),
	//	_objectBindings(info.objectBindings),
	//	_globalBindings(info.globalBindings)
	//{
	//	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	//	u32 vsCbvReg = 0, vsSrvReg = 0;
	//	rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, vsCbvReg++));
	//	rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_VERTEX, vsCbvReg++));
	//	rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX, vsSrvReg++));
	//	rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, D3D12_SHADER_VISIBILITY_VERTEX, vsSrvReg++));

	//	u32 psCbvReg = 0, psSrvReg = 0;
	//	std::vector<D3D12_DESCRIPTOR_RANGE> ranges;
	//	ranges.push_back(CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, psSrvReg++)); // diffuse
	//	ranges.push_back(CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, psSrvReg++)); // specular
	//	ranges.push_back(CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1u, psSrvReg++)); // light
	//	rootParameters.push_back(CreateDescriptorTable(ranges[0], D3D12_SHADER_VISIBILITY_PIXEL)); // diffuse
	//	rootParameters.push_back(CreateDescriptorTable(ranges[1], D3D12_SHADER_VISIBILITY_PIXEL)); // specular
	//	rootParameters.push_back(CreateDescriptorTable(ranges[2], D3D12_SHADER_VISIBILITY_PIXEL)); // light
	//	rootParameters.push_back(CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_PIXEL, psCbvReg++));

	//	auto samplerDesc = CreateStaticSampler(D3D12_FILTER_MIN_MAG_MIP_LINEAR, 0u);
	//	_rootSignature = CreateRootSignature(rootParameters, samplerDesc);
	//	_pipelineState = CreateGraphicsPipelineState(
	//		{ info.vsPath, "", "", "", info.psPath },
	//		DXGI_FORMAT_D32_FLOAT,
	//		{ DXGI_FORMAT_R8G8B8A8_UNORM }
	//	);
	//}

	//ID3DBlob* RenderPass::LoadCSO(const std::string& filepath)
	//{
	//	std::ifstream file(filepath, std::ios::binary);

	//	if (!file.is_open())
	//		throw std::runtime_error("Failed to open CSO file");

	//	file.seekg(0, std::ios_base::end);
	//	u64 size = static_cast<u64>(file.tellg());
	//	file.seekg(0, std::ios_base::beg);

	//	ID3DBlob* toReturn = nullptr;
	//	D3DCreateBlob(size, &toReturn) >> hrVerify;

	//	file.read(static_cast<char*>(toReturn->GetBufferPointer()), size);
	//	file.close();

	//	return toReturn;
	//}

	//// from hello tri
	//D3D12_ROOT_PARAMETER RenderPass::CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE typeOfView,
	//	D3D12_SHADER_VISIBILITY visibleShader, u32 shaderRegister, u32 registerSpace)
	//{
	//	D3D12_ROOT_PARAMETER toReturn = {};
	//	toReturn.ParameterType = typeOfView;
	//	toReturn.ShaderVisibility = visibleShader;
	//	toReturn.Descriptor.ShaderRegister = shaderRegister;
	//	toReturn.Descriptor.RegisterSpace = registerSpace;
	//	return toReturn;
	//}
	//D3D12_DESCRIPTOR_RANGE RenderPass::CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type,
	//	u32 nrOfDescriptors, u32 baseShaderRegister, u32 registerSpace)
	//{
	//	D3D12_DESCRIPTOR_RANGE toReturn = {};
	//	toReturn.RangeType = type;
	//	toReturn.NumDescriptors = nrOfDescriptors;
	//	toReturn.BaseShaderRegister = baseShaderRegister;
	//	toReturn.RegisterSpace = registerSpace;
	//	toReturn.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//	return toReturn;
	//}
	//D3D12_ROOT_PARAMETER RenderPass::CreateDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>& ranges, D3D12_SHADER_VISIBILITY visibleShader)
	//{
	//	// Do not use initializer list temporary vectors as input to this function
	//	// Memory needs to be valid after returning
	//	D3D12_ROOT_PARAMETER toReturn = {};
	//	toReturn.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//	toReturn.ShaderVisibility = visibleShader;
	//	toReturn.DescriptorTable.NumDescriptorRanges = static_cast<u32>(ranges.size());
	//	toReturn.DescriptorTable.pDescriptorRanges = ranges.size() != 0 ? ranges.data() : nullptr;
	//	return toReturn;
	//}
	//D3D12_ROOT_PARAMETER RenderPass::CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE& range, D3D12_SHADER_VISIBILITY visibleShader)
	//{
	//	// Do not use initializer list temporary vectors as input to this function
	//	// Memory needs to be valid after returning
	//	D3D12_ROOT_PARAMETER toReturn = {};
	//	toReturn.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//	toReturn.ShaderVisibility = visibleShader;
	//	toReturn.DescriptorTable.NumDescriptorRanges = 1u;
	//	toReturn.DescriptorTable.pDescriptorRanges = &range;
	//	return toReturn;
	//}

	//D3D12_STATIC_SAMPLER_DESC RenderPass::CreateStaticSampler(D3D12_FILTER filter, UINT shaderRegister,
	//	D3D12_SHADER_VISIBILITY visibleShader, UINT registerSpace)
	//{
	//	D3D12_STATIC_SAMPLER_DESC toReturn = {};
	//	toReturn.Filter = filter;
	//	toReturn.AddressU = toReturn.AddressV = toReturn.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	//	toReturn.MipLODBias = 0.0f;
	//	toReturn.MaxAnisotropy = 16;
	//	toReturn.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	//	toReturn.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	//	toReturn.MinLOD = 0;
	//	toReturn.MaxLOD = D3D12_FLOAT32_MAX;
	//	toReturn.ShaderRegister = shaderRegister;
	//	toReturn.RegisterSpace = registerSpace;
	//	toReturn.ShaderVisibility = visibleShader;
	//	return toReturn;
	//}
	//ID3D12RootSignature* RenderPass::CreateRootSignature(
	//	const std::vector<D3D12_ROOT_PARAMETER>& rootParameters, const D3D12_STATIC_SAMPLER_DESC& staticSampler)
	//{
	//	D3D12_ROOT_SIGNATURE_DESC desc = {};
	//	desc.NumParameters = static_cast<u32>(rootParameters.size());
	//	desc.pParameters = rootParameters.size() == 0 ? nullptr : rootParameters.data();
	//	desc.NumStaticSamplers = 1u;
	//	desc.pStaticSamplers = &staticSampler;
	//	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	//	ID3DBlob* rootSignatureBlob = nullptr;
	//	ID3DBlob* errorBlob = nullptr;
	//	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSignatureBlob, &errorBlob);

	//	ID3D12RootSignature* toReturn;
	//	_device->CreateRootSignature(0u, rootSignatureBlob->GetBufferPointer(),
	//		rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&toReturn)) >> hrVerify;

	//	rootSignatureBlob->Release();

	//	return toReturn;
	//}

	//ID3D12PipelineState* RenderPass::CreateGraphicsPipelineState(
	//	const std::array<std::string, 5>& shaderPaths, DXGI_FORMAT dsvFormat, const std::vector<DXGI_FORMAT>& rtvFormats)
	//{
	//	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
	//	ZeroMemory(&desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	//	desc.pRootSignature = _rootSignature;
	//	std::array<D3D12_SHADER_BYTECODE*, 5> shaders = { &desc.VS, &desc.HS, &desc.DS, &desc.GS, &desc.PS };
	//	std::array<ID3DBlob*, 5> shaderBlobs;

	//	for (int i = 0; i < 5; ++i)
	//	{
	//		if (shaderPaths[i] != "")
	//		{
	//			shaderBlobs[i] = LoadCSO(shaderPaths[i]);
	//			(*shaders[i]).pShaderBytecode = shaderBlobs[i]->GetBufferPointer();
	//			(*shaders[i]).BytecodeLength = shaderBlobs[i]->GetBufferSize();
	//		}
	//	}

	//	desc.SampleMask = u32(-1);
	//	desc.RasterizerState = CreateRasterizerDesc();
	//	desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//	desc.NumRenderTargets = static_cast<u32>(rtvFormats.size());

	//	desc.BlendState.AlphaToCoverageEnable = false;
	//	desc.BlendState.IndependentBlendEnable = false;

	//	for (unsigned int i = 0; i < rtvFormats.size(); ++i)
	//	{
	//		desc.RTVFormats[i] = rtvFormats[i];
	//		desc.BlendState.RenderTarget[i] = CreateBlendDesc();
	//	}

	//	desc.SampleDesc.Count = 1;
	//	desc.SampleDesc.Quality = 0;
	//	desc.DSVFormat = dsvFormat;
	//	desc.DepthStencilState = CreateDepthStencilDesc();
	//	desc.StreamOutput = CreateStreamOutputDesc();
	//	desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	//	ID3D12PipelineState* toReturn;
	//	_device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&toReturn)) >> hrVerify;

	//	return toReturn;
	//}

	//const std::vector<PipelineBinding>& RenderPass::GetObjectBindings()
	//{
	//	return _objectBindings;
	//}
	//const std::vector<PipelineBinding>& RenderPass::GetGlobalBindings()
	//{
	//	return _globalBindings;
	//}

	//ID3D12RootSignature* RenderPass::GetRootSignature()
	//{
	//	return _rootSignature;
	//}
	//ID3D12PipelineState* RenderPass::GetPipelineState()
	//{
	//	return _pipelineState;
	//}

	//D3D12_RASTERIZER_DESC RenderPass::CreateRasterizerDesc()
	//{
	//	D3D12_RASTERIZER_DESC toReturn{};
	//	toReturn.FillMode = D3D12_FILL_MODE_SOLID;
	//	toReturn.CullMode = D3D12_CULL_MODE_BACK;
	//	toReturn.FrontCounterClockwise = false;
	//	toReturn.DepthBias = 0;
	//	toReturn.DepthBiasClamp = 0.0f;
	//	toReturn.SlopeScaledDepthBias = 0.0f;
	//	toReturn.DepthClipEnable = true;
	//	toReturn.MultisampleEnable = false;
	//	toReturn.AntialiasedLineEnable = false;
	//	toReturn.ForcedSampleCount = 0;
	//	toReturn.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	//	return toReturn;
	//}
	//D3D12_RENDER_TARGET_BLEND_DESC RenderPass::CreateBlendDesc()
	//{
	//	D3D12_RENDER_TARGET_BLEND_DESC toReturn{};
	//	toReturn.BlendEnable = false;
	//	toReturn.LogicOpEnable = false;
	//	toReturn.SrcBlend = D3D12_BLEND_ONE;
	//	toReturn.DestBlend = D3D12_BLEND_ZERO;
	//	toReturn.BlendOp = D3D12_BLEND_OP_ADD;
	//	toReturn.SrcBlendAlpha = D3D12_BLEND_ONE;
	//	toReturn.DestBlendAlpha = D3D12_BLEND_ZERO;
	//	toReturn.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//	toReturn.LogicOp = D3D12_LOGIC_OP_NOOP;
	//	toReturn.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//	return toReturn;
	//}
	//D3D12_DEPTH_STENCIL_DESC RenderPass::CreateDepthStencilDesc()
	//{
	//	D3D12_DEPTH_STENCIL_DESC toReturn{};
	//	toReturn.DepthEnable = true;
	//	toReturn.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	//	toReturn.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	//	toReturn.StencilEnable = false;
	//	toReturn.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	//	toReturn.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	//	toReturn.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	toReturn.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
	//	toReturn.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	toReturn.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
	//	toReturn.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//	toReturn.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
	//	toReturn.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//	toReturn.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	//	return toReturn;
	//}
	//D3D12_STREAM_OUTPUT_DESC RenderPass::CreateStreamOutputDesc()
	//{
	//	D3D12_STREAM_OUTPUT_DESC toReturn = {};
	//	toReturn.pSODeclaration = nullptr;
	//	toReturn.NumEntries = 0;
	//	toReturn.pBufferStrides = nullptr;
	//	toReturn.NumStrides = 0;
	//	toReturn.RasterizedStream = 0;
	//	return toReturn;
	//}

	//RenderPass::~RenderPass()
	//{
	//	_rootSignature->Release();
	//	_pipelineState->Release();
	//}

}
