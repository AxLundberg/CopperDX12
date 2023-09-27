#pragma once
#include "cmn/D12Headers.h"
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
		TextureManager* _textureMan = nullptr;
		SamplerManager* _samplerMan = nullptr;
		BufferManager* _bufferMan = nullptr;
		HeapManager* _heapMan = nullptr;
		RenderPass* _currentPass = nullptr;

		static constexpr u32 BUFFER_COUNT = 2;
		// Microsoft::WRL::ComPtr
		ComPtr<ID3D12Device5> _device;
		ComPtr<IDXGIFactory6> _factory;
		ComPtr<IDXGIAdapter> _adapter;
		ComPtr<ID3D12CommandQueue> _cmdQ;
		ComPtr<ID3D12CommandAllocator> _cmdAllo;
		ComPtr<ID3D12GraphicsCommandList> _cmdList;
		ComPtr<IDXGISwapChain4> _swapchain;
		ComPtr<ID3D12DescriptorHeap> _rtvDescHeap;
		ComPtr<ID3D12Resource> _backBuffers[BUFFER_COUNT];
		ComPtr<ID3D12Fence> _fence;
	};
}
