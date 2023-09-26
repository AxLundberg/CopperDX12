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
		Renderer(HWND);
		ResourceIndex CreateSampler(SamplerType, AddressMode) override;
		void CreateRenderPass(RenderPassInfo&) override;
		ResourceIndex SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding) override;
		ResourceIndex SubmitTexture(void* data, TextureInfo&) override;

		void PreRender() override;
		void Render() override;
		void Present() override;
		~Renderer() override;

	private:
		static constexpr u32 BUFFER_COUNT = 2;
		Microsoft::WRL::ComPtr<ID3D12Device5> _device;
		Microsoft::WRL::ComPtr<IDXGIFactory6> _factory;
		Microsoft::WRL::ComPtr<IDXGIAdapter> _adapter;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> _cmdQ;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _cmdAllo;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> _swapchain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvDescHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> _backBuffers[BUFFER_COUNT];
	};
}
