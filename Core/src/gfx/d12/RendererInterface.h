#pragma once
#include <Core/src/win/IWindow.h>

#include "../cmn/TypeDefs.h"

#include "ResourceManager.h"
#include "TextureManager.h"
#include "SamplerManager.h"
#include "BufferManager.h"
#include "RenderObject.h"
#include "HeapManager.h"
#include "RenderPass.h"
#include "Camera.h"

namespace CPR::GFX::D12
{
	class IRenderer
	{
	public:
		virtual ~IRenderer() = default;
		
		virtual ResourceIndex CreateSampler(SamplerType, AddressMode) = 0;
		virtual RenderPass* CreateRenderPass(RenderPassInfo&) = 0;
		virtual ResourceIndex SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding) = 0;
		virtual ResourceIndex SubmitTexture(void* data, TextureInfo&) = 0;
		virtual Camera* CreateCamera(float minDepth, float maxDepth, float aspectRatio) = 0;
		virtual void UpdateBuffer(ResourceIndex bufferIndex, void* data) = 0;
		virtual void SetLightBuffer(ResourceIndex lightBufferIndexToUse) = 0;

		virtual void Initialize(HWND) = 0;
		virtual void SetRenderPass(RenderPass* toSet) = 0;
		virtual void PreRender() = 0;
		virtual void Render(const std::vector<RenderObject>& objectsToRender) = 0;
		virtual void Present() = 0;
	private:
	};
}