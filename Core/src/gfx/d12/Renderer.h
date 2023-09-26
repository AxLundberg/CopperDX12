#pragma once
#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

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
		~Renderer() override;

	private:
		Microsoft::WRL::ComPtr<ID3D12Device5> _device;
		Microsoft::WRL::ComPtr<IDXGIFactory6> _factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> _adapter;
	};
}
