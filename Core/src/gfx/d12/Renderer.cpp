#include "Renderer.h"
#include "cmn/RunConfig.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	Renderer::Renderer(HWND window)
	{
		using Microsoft::WRL::ComPtr;

		if(_DEBUG_)
		{
			ComPtr<ID3D12Debug> debugController;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) >> hrVerify;
			debugController->EnableDebugLayer();
		}

		// Initialize factory, adapter and device
		{
			u32 factoryFlags = _DEBUG_ ? DXGI_CREATE_FACTORY_DEBUG : 0;
			CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&_factory)) >> hrVerify;

			_factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&_adapter)) >> hrVerify;

			D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)) >> hrVerify;
			u32 adapterIndex = 0;
		}

		// command queue
		{
			const D3D12_COMMAND_QUEUE_DESC desc =
			{
				.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0,
			};
			_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmdQ)) >> hrVerify;
		}

		// swap chain
		{
			const DXGI_SWAP_CHAIN_DESC1 desc = {
				.Width = 900,
				.Height = 600,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.Stereo = FALSE,
				.SampleDesc = {
					.Count = 1,
					.Quality = 0,
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = BUFFER_COUNT,
				.Scaling = DXGI_SCALING_STRETCH,
				.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
				.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
				.Flags = 0,
			};
			ComPtr<IDXGISwapChain1> swapChain1;
			_factory->CreateSwapChainForHwnd(
				_cmdQ.Get(),
				window,
				&desc,
				nullptr,
				nullptr,
				&swapChain1) >> hrVerify;
			swapChain1.As(&_swapchain) >> hrVerify;
		}

		// rtv descriptor heap
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = BUFFER_COUNT,
			};
			_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvDescHeap)) >> hrVerify;
		}
		const auto rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// rtv desciptors and buffer references
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
			for (u32 i = 0; i < BUFFER_COUNT; ++i)
			{
				_swapchain->GetBuffer(i, IID_PPV_ARGS(&_backBuffers[i])) >> hrVerify;
				_device->CreateRenderTargetView(_backBuffers[i].Get(), nullptr, rtvHandle);
				rtvHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			}
		}

		// command allocator and list
		{
			_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllo)) >> hrVerify;

			_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				_cmdAllo.Get(), nullptr, IID_PPV_ARGS(&_cmdList)) >> hrVerify;
			_cmdList->Close() >> hrVerify;
		}

		//_samplerMan = new SamplerManager(_device);
		//_heapMan = new HeapManager(_device, _cmdList);
		//_bufferMan = new BufferManager(_device, _heapMan);
		//_textureMan = new TextureManager(_device, _heapMan);

	}
	ResourceIndex Renderer::CreateSampler(SamplerType, AddressMode)
	{
		return ResourceIndex();
	}
	void Renderer::CreateRenderPass(RenderPassInfo&)
	{
	}
	ResourceIndex Renderer::SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, BufferBinding)
	{
		return ResourceIndex();
	}
	ResourceIndex Renderer::SubmitTexture(void* data, TextureInfo&)
	{
		return ResourceIndex();
	}
	void Renderer::PreRender()
	{
	}
	void Renderer::Render()
	{
	}
	void Renderer::Present()
	{
	}
	Renderer::~Renderer()
	{
		delete _bufferMan;
		delete _heapMan;
	}
}
