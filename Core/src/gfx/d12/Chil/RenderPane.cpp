#include "RenderPane.h"
#include <Core/src/gfx/cmn/GraphicsError.h>
#include <Core/src/log/Log.h>
#include <Core/src/utl/String.h>
#include "ResourceLoader.h"
#include <d3dcompiler.h>
#include <ranges>

namespace CPR::GFX::D12
{
	using Microsoft::WRL::ComPtr;
	namespace rn = std::ranges;

	RenderPane::RenderPane(HWND hWnd, const SPA::DimensionsI& dims, std::shared_ptr<IDeviceChil> pDevice,
		std::shared_ptr<ICommandQueue> pCommandQueue)
		:
		dims_{ dims },
		pDevice_{ std::move(pDevice) },
		pCommandQueue_{ std::move(pCommandQueue) }
	{
		// cache device interface
		auto pDeviceInterface = pDevice_->GetD3D12DeviceInterface();
		// create swap chain
		{
			const DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
				.Width = (UINT)dims.width,
				.Height = (UINT)dims.height,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.Stereo = FALSE,
				.SampleDesc = {
					.Count = 1,
					.Quality = 0
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = bufferCount_,
				.Scaling = DXGI_SCALING_STRETCH,
				.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
				.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
				.Flags = 0,
			};
			ComPtr<IDXGISwapChain1> swapChain1;
			pDevice_->GetDXGIFactoryInterface()->CreateSwapChainForHwnd(
				pCommandQueue_->GetD3D12CommandQueue().Get(),
				hWnd,
				&swapChainDesc,
				nullptr,
				nullptr,
				&swapChain1) >> hrVerify;
			swapChain1.As(&pSwapChain_) >> hrVerify;
		}
		// create rtv descriptor heap
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = bufferCount_,
			};
			pDeviceInterface->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pRtvDescriptorHeap_)) >> hrVerify;
		}
		// cache rtv descriptor size
		rtvDescriptorSize_ = pDeviceInterface->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		// create rtvs and get resource handles for each buffer in the swap chain
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(
				pRtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
			for (int i = 0; i < bufferCount_; i++) {
				pSwapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i])) >> hrVerify;
				pDeviceInterface->CreateRenderTargetView(backBuffers_[i].Get(), nullptr, rtvHandle);
				rtvHandle.ptr += rtvDescriptorSize_;
			}
		}
		// depth buffer 
		{
			D3D12_HEAP_PROPERTIES heapProperties = {};
			ZeroMemory(&heapProperties, sizeof(heapProperties));
			heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
			desc.Width = static_cast<u32>(dims.width);
			desc.Height = static_cast<u32>(dims.height);
			desc.DepthOrArraySize = 1;
			desc.MipLevels = 1u;
			desc.Format = DXGI_FORMAT_D32_FLOAT;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
			desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			const D3D12_CLEAR_VALUE clearValue = {
				.Format = DXGI_FORMAT_D32_FLOAT,
				.DepthStencil = { 1.0f, 0 },
			};
			pDeviceInterface->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&clearValue,
				IID_PPV_ARGS(&pDepthBuffer_)) >> hrVerify;
		}
		// dsv descriptor heap 
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1,
			};
			pDeviceInterface->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&pDsvDescriptorHeap_)) >> hrVerify;
		}
		// dsv and handle 
		{
			const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{
				pDsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() };
			pDeviceInterface->CreateDepthStencilView(pDepthBuffer_.Get(), nullptr, dsvHandle);
		}
		// scissor rect
		scissorRect_ = { 0, 0, LONG_MAX, LONG_MAX };
		// viewport
		viewport_ = { 0, 0, static_cast<f32>(dims.width), static_cast<f32>(dims.height), 0.0f, 1.0f };
	}

	RenderPane::~RenderPane()
	{
		// wait for queue to become completely empty
		pCommandQueue_->Flush();
	}

	void RenderPane::BeginFrame()
	{
		// set index of swap chain buffer for this frame
		curBackBufferIndex_ = pSwapChain_->GetCurrentBackBufferIndex();
		// wait for this back buffer to become free
		pCommandQueue_->WaitForFenceValue(bufferFenceValues_[curBackBufferIndex_]);
		// acquire command list
		auto commandListPair = pCommandQueue_->GetAllocatorListPair();
		// transition buffer resource to render target state 
		auto& backBuffer = backBuffers_[curBackBufferIndex_];
		TransitionResource(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		// clear back buffer
		const auto rtvDescSize = pDevice_->GetD3D12DeviceInterface()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		if (clearColor_) {
			D3D12_CPU_DESCRIPTOR_HANDLE rtv{
				pRtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() };
			rtv.ptr += rtvDescriptorSize_ * curBackBufferIndex_;
			commandListPair.pCommandList->ClearRenderTargetView(rtv, &clearColor_->x, 0, nullptr);
		}
		// clear the depth buffer 
		const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{
			pDsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() };
		commandListPair.pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
		// execute begin frame commands
		pCommandQueue_->ExecuteCommandList(std::move(commandListPair));
	}

	AllocatorListPair RenderPane::GetCommandList()
	{
		// acquire command list
		auto commandListPair = pCommandQueue_->GetAllocatorListPair();
		// configure RS 
		commandListPair.pCommandList->RSSetViewports(1, &viewport_);
		commandListPair.pCommandList->RSSetScissorRects(1, &scissorRect_);
		// bind render target and depth buffer
		const D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{
			pDsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() };
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{
				pRtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() };
		rtvHandle.ptr += rtvDescriptorSize_ * curBackBufferIndex_;
		commandListPair.pCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

		return commandListPair;
	}

	void RenderPane::SubmitCommandList(AllocatorListPair commands)
	{
		pCommandQueue_->ExecuteCommandList(std::move(commands));
	}

	uint64_t RenderPane::GetFrameFenceValue() const
	{
		return pCommandQueue_->GetFrameFenceValue();
	}

	uint64_t RenderPane::GetSignalledFenceValue() const
	{
		return pCommandQueue_->GetSignalledFenceValue();
	}

	void RenderPane::EndFrame()
	{
		auto& backBuffer = backBuffers_[curBackBufferIndex_];

		// get a command list for end frame commands
		auto commandListPair = pCommandQueue_->GetAllocatorListPair();
		// prepare buffer for presentation by transitioning to present state
		TransitionResource(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		// submit command list 
		pCommandQueue_->ExecuteCommandList(std::move(commandListPair));
		// present frame 
		pSwapChain_->Present(1, 0) >> hrVerify;
		// insert a fence so we know when the buffer is free
		bufferFenceValues_[curBackBufferIndex_] = pCommandQueue_->SignalFrameFence();
	}

	void RenderPane::FlushQueues() const
	{
		pCommandQueue_->Flush();
	}

	void RenderPane::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES newState)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = currentState;
		barrier.Transition.StateAfter = newState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		auto commandListPair = pCommandQueue_->GetAllocatorListPair();
		commandListPair.pCommandList->ResourceBarrier(1, &barrier);
	}

}