#pragma once
#include <Core/src/utl/String.h>

#include "cmn/D12Headers.h"
#include "DeviceD12.h"


namespace CPR::GFX::D12
{
	struct RenderPassInfo;

	class PipelineState
	{
	public:
		PipelineState(IDevice* device, const std::array<std::string, 5> shaders, ID3D12RootSignature*);
		~PipelineState();
		operator ID3D12PipelineState* () const { return _pipelineState.Get(); }
	private:
		ID3DBlob* LoadCSO(const std::string& filepath);
		D3D12_RASTERIZER_DESC CreateRasterizerDesc();
		D3D12_RENDER_TARGET_BLEND_DESC CreateBlendDesc();
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
		D3D12_STREAM_OUTPUT_DESC CreateStreamOutputDesc();
	private:
		ComPtr<ID3D12PipelineState> _pipelineState;
	};

}
