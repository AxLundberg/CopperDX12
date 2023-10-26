#pragma once
#include <Core/src/utl/String.h>
#include <vector>
#include <array>

#include "PipelineState.h"
#include "RootSignature.h"
#include "cmn/D12Headers.h"
#include "../cmn/TypeDefs.h"
#include "../IDevice.h"

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

	class GfxRenderPass
	{
	public:
		GfxRenderPass(IDevice* device, const RenderPassInfo& info);

		void SetGlobalSampler(PipelineShaderStage shader,
			std::uint8_t slot, ResourceIndex index);
		ResourceIndex GetGlobalSampler(PipelineShaderStage shader,
			std::uint8_t slot) const;

		const std::vector<PipelineBinding>& GetObjectBindings() { return _objectBindings; };
		const std::vector<PipelineBinding>& GetGlobalBindings() { return _globalBindings; };

		RootSignature& GetRootSignature() { return _rootSignature; };
		PipelineState& GetPipelineState() { return _pipelineState; };
	private:
		RootSignature _rootSignature;
		PipelineState _pipelineState;
		std::vector<PipelineBinding> _objectBindings;
		std::vector<PipelineBinding> _globalBindings;
		std::array<ResourceIndex, D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT> _vsGlobalSamplers{};
		std::array<ResourceIndex, D3D12_COMMONSHADER_SAMPLER_SLOT_COUNT> _psGlobalSamplers{};
	};
}
