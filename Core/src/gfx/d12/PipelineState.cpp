#include <array>
#include <fstream>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "PipelineState.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	PipelineState::PipelineState(IDevice* d, const std::array<std::string, 5> shaderPaths, ID3D12RootSignature* rootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
		ZeroMemory(&desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
		desc.pRootSignature = rootSignature;
		std::array<D3D12_SHADER_BYTECODE*, 5> shaders = { &desc.VS, &desc.HS, &desc.DS, &desc.GS, &desc.PS };
		std::array<ID3DBlob*, 5> shaderBlobs;

		//const std::array<std::string, 5> shaderPaths = shaderNames; // AXEL
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
}
