#pragma once
#include <Core/src/utl/String.h>
#include <vector>
#include <array>

#include "../cmn/TypeDefs.h"
#include "cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	enum class PipelineDataType
	{
		NONE,
		TRANSFORM,
		VIEW_PROJECTION,
		CAMERA_POS,
		LIGHT,
		VERTEX,
		INDEX,
		DIFFUSE,
		SPECULAR,
		SAMPLER
	};

	enum class PipelineBindingType
	{
		NONE,
		CONSTANT_BUFFER,
		SHADER_RESOURCE,
		UNORDERED_ACCESS
	};

	enum class PipelineShaderStage
	{
		NONE,
		VS,
		PS
	};

	struct PipelineBinding
	{
		PipelineDataType dataType = PipelineDataType::NONE;
		PipelineBindingType bindingType = PipelineBindingType::NONE;
		PipelineShaderStage shaderStage = PipelineShaderStage::NONE;
		std::uint8_t slotToBindTo = std::uint8_t(-1);
	};

	struct RenderPassInfo
	{
		std::string vsPath = "";
		std::string psPath = "";
		std::vector<PipelineBinding> objectBindings;
		std::vector<PipelineBinding> globalBindings;
	};

	class RenderPass
	{
	public:
		RenderPass(ComPtr<ID3D12Device5> const& device, const RenderPassInfo& info);
		~RenderPass();
		
		const std::vector<PipelineBinding>& GetObjectBindings();
		const std::vector<PipelineBinding>& GetGlobalBindings();

		void SetGlobalSampler(PipelineShaderStage shader,
			std::uint8_t slot, ResourceIndex index);
		ResourceIndex GetGlobalSampler(PipelineShaderStage shader,
			std::uint8_t slot) const;

		ID3D12RootSignature* GetRootSignature();
		ID3D12PipelineState* GetPipelineState();

	private:
		// from hello tri
		ID3DBlob* LoadCSO(const std::string& filepath);
		D3D12_ROOT_PARAMETER CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE typeOfView,
			D3D12_SHADER_VISIBILITY visibleShader, u32 shaderRegister, u32 registerSpace = 0);
		D3D12_DESCRIPTOR_RANGE CreateDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type,
			u32 nrOfDescriptors, u32 baseShaderRegister, u32 registerSpace = 0);
		D3D12_ROOT_PARAMETER CreateDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE>&, D3D12_SHADER_VISIBILITY);
		D3D12_ROOT_PARAMETER CreateDescriptorTable(D3D12_DESCRIPTOR_RANGE&, D3D12_SHADER_VISIBILITY);
		D3D12_STATIC_SAMPLER_DESC CreateStaticSampler(D3D12_FILTER filter, UINT shaderRegister,
			D3D12_SHADER_VISIBILITY visibleShader = D3D12_SHADER_VISIBILITY_PIXEL, UINT registerSpace = 0);
		ID3D12RootSignature* CreateRootSignature(const std::vector<D3D12_ROOT_PARAMETER>& rootParameters, const D3D12_STATIC_SAMPLER_DESC& staticSampler);
		ID3D12PipelineState* CreateGraphicsPipelineState(const std::array<std::string, 5>& shaderPaths, DXGI_FORMAT dsvFormat, const std::vector<DXGI_FORMAT>& rtvFormats);

		D3D12_RASTERIZER_DESC CreateRasterizerDesc();
		D3D12_RENDER_TARGET_BLEND_DESC CreateBlendDesc();
		D3D12_DEPTH_STENCIL_DESC CreateDepthStencilDesc();
		D3D12_STREAM_OUTPUT_DESC CreateStreamOutputDesc();
	private:
		ComPtr<ID3D12Device5> _device;
		
		std::array<ResourceIndex, D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT> _vsGlobalSamplers;
		std::array<ResourceIndex, D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT> _psGlobalSamplers;

		std::vector<PipelineBinding> _objectBindings;
		std::vector<PipelineBinding> _globalBindings;
		
		ID3D12PipelineState* _pipelineState;
		ID3D12RootSignature* _rootSignature;
	};
}
