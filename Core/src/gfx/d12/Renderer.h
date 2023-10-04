#pragma once
#include "cmn/D12Headers.h"
#include "IRenderer.h"
#include "experiment/SyncCommander.h"
#include "experiment/DescriptorHeap.h"

namespace CPR::GFX::D12
{
	class Renderer : public IRenderer
	{
	private:
		static constexpr u32 BACKBUFFER_COUNT = 2;
		static constexpr u32 DESCRIPTOR_HEAP_SIZE = 1000;
		static constexpr f32 CLEAR_COLOR[4] = { 0.1f, 0.1f, 0.1f, 0.0f };
	public:
		Renderer(std::shared_ptr<CPR::GFX::IDevice> device,
			std::shared_ptr<CPR::GFX::ISwapChain> swapChain,
			std::shared_ptr<CPR::GFX::D12::ISyncCommander> syncCommander);

		~Renderer();

		ResourceIndex CreateSampler(SamplerType, AddressMode) override;
		GfxRenderPass* CreateRenderPass(RenderPassInfo&) override;
		ResourceIndex SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding) override;
		ResourceIndex SubmitTexture(void* data, TextureInfo&) override;
		Camera* CreateCamera(float minDepth, float maxDepth, float aspectRatio) override;
		void SetLightBuffer(ResourceIndex lightBufferIndexToUse) override;
		void UpdateBuffer(ResourceIndex bufferIndex, void* data) override;

		void Initialize(HWND) override;
		void SetRenderPass(GfxRenderPass* toSet) override;
		void PreRender() override;
		void Render(const std::vector<RenderObject>& objectsToRender) override;
		void Present() override;
	private:
		std::shared_ptr<CPR::GFX::IDevice> GetDevice() override;

		void ExecuteCommandList();
		void FlushCommandQueue();
		void ResetCommandMemory();
		void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES newState);
	private:
		std::shared_ptr<CPR::GFX::D12::ISyncCommander> _syncMan;
		std::shared_ptr<CPR::GFX::IDevice> _device;
		std::shared_ptr<CPR::GFX::ISwapChain> _swapChain;
		GfxRenderPass* _currentPass = nullptr;
		Camera* _camera = nullptr;
		TextureManager* _textureMan = nullptr;
		SamplerManager* _samplerMan = nullptr;
		BufferManager* _bufferMan = nullptr;
		HeapManager* _heapMan = nullptr;
		//RenderPass* _currentPass = nullptr;
		DescriptorHeap _bindableDescHeap;
		DescriptorHeap _rtvDescHeap;
		DescriptorHeap _dsvDescHeap;

		// Microsoft::WRL::ComPtr
		//ComPtr<ID3D12Device5> _device;
		ComPtr<IDXGIFactory6> _factory;
		ComPtr<IDXGIAdapter> _adapter;
		ComPtr<ID3D12CommandQueue> _cmdQ;
		ComPtr<ID3D12CommandAllocator> _cmdAllo;
		ComPtr<ID3D12GraphicsCommandList4> _cmdList;
		ComPtr<IDXGISwapChain4> _swapchain;
		//ComPtr<ID3D12DescriptorHeap> _rtvDescHeap;
		//ComPtr<ID3D12DescriptorHeap> _dsvDescHeap;
		//ComPtr<ID3D12DescriptorHeap> _bindableDescHeap;
		ComPtr<ID3D12Resource> _depthStencil;
		ComPtr<ID3D12Resource> _backbuffers[BACKBUFFER_COUNT];
		ComPtr<ID3D12Fence> _fence;

		u64 _currentFenceValue = 0u;
		u64 _currentBackbuffer = BACKBUFFER_COUNT - 1;
		ResourceIndex _lightBufferIndex = ResourceIndex(-1);
	};
}
