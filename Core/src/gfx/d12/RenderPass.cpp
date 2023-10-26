#include "RenderPass.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	GfxRenderPass::GfxRenderPass(IDevice* device, const RenderPassInfo& info)
		: 
		_rootSignature(device),
		_pipelineState(device, { info.vsPath, "", "", "", info.psPath }, _rootSignature),
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
			break;
			//throw std::runtime_error("Incorrect shader stage when setting sampler");
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
			break;
			//throw std::runtime_error("Incorrect shader stage when fetching sampler");
		}
	}
}
