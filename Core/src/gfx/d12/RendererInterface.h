#pragma once
#include "../cmn/TypeDefs.h"

#include "ResourceManager.h"
#include "TextureManager.h"
#include "SamplerManager.h"
#include "BufferManager.h"
#include "HeapManager.h"
#include "RenderPass.h"

namespace CPR::GFX::D12
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;

		virtual ResourceIndex CreateSampler(SamplerType, AddressMode) = 0;
		virtual void CreateRenderPass(RenderPassInfo&) = 0;
		virtual ResourceIndex SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding) = 0;
		virtual ResourceIndex SubmitTexture(void* data, TextureInfo&) = 0;

		virtual void PreRender() = 0;
		virtual void Render() = 0;
		virtual void Present() = 0;
	private:
	};
}