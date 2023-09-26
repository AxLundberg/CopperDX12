#pragma once
#include "RendererInterface.h"

namespace CPR::GFX::D12
{
	class Renderer : IRenderer
	{
	public:
		Renderer();
		ResourceIndex CreateSampler(SamplerType, AddressMode) override;
		void CreateRenderPass(RenderPassInfo&) override;
		ResourceIndex SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding) override;
		ResourceIndex SubmitTexture(void* data, TextureInfo&) override;

		void PreRender() override;
		void Render() override;
		void Present() override;
		~Renderer();

	private:

	};

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}
}