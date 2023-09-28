#include <iostream>

#include "Renderer.h"
#include "cmn/RunConfig.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	Renderer::Renderer()
	{}

	void Renderer::Initialize(HWND window)
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

			// POCO::NotFoundException is a nvidia driver issue
			D3D12CreateDevice(_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)) >> hrVerify;
		}

		// command queue, allocator, list and fence
		{
			const D3D12_COMMAND_QUEUE_DESC desc =
			{
				.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
				.Priority = 0,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0,
			};
			_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_cmdQ)) >> hrVerify;
		
			_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllo)) >> hrVerify;

			_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
				_cmdAllo.Get(), nullptr, IID_PPV_ARGS(&_cmdList)) >> hrVerify;
			
			_device->CreateFence(_currentFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)) >> hrVerify;
		}
		
		// swap chain
		{
			const DXGI_SWAP_CHAIN_DESC1 desc = {
				.Width = 0u,
				.Height = 0u,
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.Stereo = FALSE,
				.SampleDesc = {
					.Count = 1,
					.Quality = 0,
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = BACKBUFFER_COUNT,
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

		// Bindable Descriptor Heap
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				.NumDescriptors = DESCRIPTOR_HEAP_SIZE,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			};
			_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_bindableDescHeap)) >> hrVerify;
		}

		// managers
		_samplerMan = new SamplerManager(_device);
		_heapMan = new HeapManager(_device, _cmdList);
		_bufferMan = new BufferManager(_device, _heapMan);
		_textureMan = new TextureManager(_device, _heapMan);

		constexpr u32 srvCount = 100;
		_textureMan->ReserveHeapSpace(_bindableDescHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, srvCount, 0u);
		_bufferMan->ReserveHeapSpace(_bindableDescHeap.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DESCRIPTOR_HEAP_SIZE - srvCount, srvCount);

		// rtv descriptor heap
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = BACKBUFFER_COUNT,
			};
			_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_rtvDescHeap)) >> hrVerify;
		
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
			for (u32 i = 0; i < BACKBUFFER_COUNT; ++i)
			{
				_swapchain->GetBuffer(i, IID_PPV_ARGS(&_backbuffers[i])) >> hrVerify;
				_device->CreateRenderTargetView(_backbuffers[i].Get(), nullptr, rtvHandle);
				rtvHandle.ptr += _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			}
		}
		// dsv descriptor heap
		{
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
				.NumDescriptors = 1u,
			};
			_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_dsvDescHeap)) >> hrVerify;

			D3D12_CLEAR_VALUE depthClearValue = {};
			depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			depthClearValue.DepthStencil.Depth = 1.0f;

			_depthStencil = _textureMan->CreateTexture2DResource(
				_backbuffers[0]->GetDesc().Width,
				_backbuffers[0]->GetDesc().Height,
				1u,
				DXGI_FORMAT_D32_FLOAT,
				D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE,
				&depthClearValue
			);
			
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = _depthStencil.Get();
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			_cmdList->ResourceBarrier(1, &barrier);

			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvDescHeap->GetCPUDescriptorHandleForHeapStart();
			_device->CreateDepthStencilView(_depthStencil.Get(), nullptr, dsvHandle);
		}
	}
	ResourceIndex Renderer::CreateSampler(SamplerType samplerType, AddressMode addressMode)
	{
		return _samplerMan->CreateSampler(samplerType, addressMode);
	}
	RenderPass* Renderer::CreateRenderPass(RenderPassInfo& initInfo)
	{
		_currentPass = new RenderPass(_device, initInfo);
		return _currentPass;
	}
	void Renderer::SetRenderPass(RenderPass* toSet)
	{
		_currentPass = static_cast<RenderPass*>(toSet);
	}
	ResourceIndex Renderer::SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage rwPattern, BufferBinding bufferBinding)
	{
		return _bufferMan->SubmitBuffer(data, elemSize, elemCount, rwPattern, bufferBinding);
	}
	ResourceIndex Renderer::SubmitTexture(void* data, TextureInfo& textureInfo)
	{
		return _textureMan->AddTexture(data, textureInfo);
	}
	Camera* Renderer::CreateCamera(float minDepth, float maxDepth, float aspectRatio)
	{
		_camera = new Camera(*_bufferMan, minDepth, maxDepth, aspectRatio);
		return _camera;
	}
	void Renderer::SetLightBuffer(ResourceIndex lightBufferIndexToUse)
	{
		_lightBufferIndex = lightBufferIndexToUse;
	}
	void Renderer::PreRender()
	{
		static bool firstFrame = true;
		if (firstFrame)
		{
			ExecuteCommandList();
			FlushCommandQueue();
			firstFrame = false;
		}
		ResetCommandMemory();

		_currentBackbuffer = (_currentBackbuffer + 1u) % BACKBUFFER_COUNT;
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = _rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += static_cast<size_t>(_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)) * _currentBackbuffer;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = _dsvDescHeap->GetCPUDescriptorHandleForHeapStart();

		// Record commands
		_cmdList->SetDescriptorHeaps(1, _bindableDescHeap.GetAddressOf());
		_cmdList->SetGraphicsRootSignature(_currentPass->GetRootSignature());
		_cmdList->SetPipelineState(_currentPass->GetPipelineState());

		TransitionResource(_backbuffers[_currentBackbuffer].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		);

		_cmdList->ClearRenderTargetView(rtvHandle, CLEAR_COLOR, 0, nullptr);
		_cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->OMSetRenderTargets(1, &rtvHandle, true, &dsvHandle);

		auto backBufferDesc = _backbuffers[_currentBackbuffer]->GetDesc();
		D3D12_VIEWPORT viewport = { 0, 0, static_cast<f32>(backBufferDesc.Width), static_cast<f32>(backBufferDesc.Height), 0.0f, 1.0f };
		D3D12_RECT scissorRect = { 0, 0, static_cast<long>(backBufferDesc.Width), static_cast<long>(backBufferDesc.Height) };
		_cmdList->RSSetScissorRects(1, &scissorRect);
		_cmdList->RSSetViewports(1, &viewport);
	}

	void Renderer::Render(const std::vector<RenderObject>& objectsToRender)
	{
		const std::vector<PipelineBinding>& objectBindings = _currentPass->GetObjectBindings();
		const std::vector<PipelineBinding>& globalBindings = _currentPass->GetGlobalBindings();

		enum RootParamIndex : u32
		{
			transform = 0u,
			viewProj,
			vertices,
			indices,
			diffuse,
			specular,
			light,
			camPos,
		};

		for (const auto& binding : globalBindings)
		{
			if (binding.dataType == PipelineDataType::VIEW_PROJECTION)
			{
				_cmdList->SetGraphicsRootConstantBufferView(RootParamIndex::viewProj, _bufferMan->GetResourceHandle(_camera->GetVP(*_bufferMan)));
			}
			else if (binding.dataType == PipelineDataType::CAMERA_POS)
			{
				_cmdList->SetGraphicsRootConstantBufferView(RootParamIndex::camPos, _bufferMan->GetResourceHandle(_camera->GetPosition(*_bufferMan)));
			}
			else if (binding.dataType == PipelineDataType::LIGHT)
			{
				_cmdList->SetGraphicsRootDescriptorTable(RootParamIndex::light, _bufferMan->GetDescriptorHandle(_lightBufferIndex));
			}
			else if (binding.dataType == PipelineDataType::SAMPLER)
			{
				continue; // Only static sampler
			}
		}

		for (const auto& object : objectsToRender)
		{
			for (auto& binding : objectBindings)
			{
				if (binding.dataType == PipelineDataType::TRANSFORM)
				{
					_cmdList->SetGraphicsRootConstantBufferView(RootParamIndex::transform, _bufferMan->GetResourceHandle(object.transformBuffer));
				}
				else if (binding.dataType == PipelineDataType::VERTEX)
				{
					_cmdList->SetGraphicsRootShaderResourceView(RootParamIndex::vertices, _bufferMan->GetResourceHandle(object.mesh.vertexBuffer));
				}
				else if (binding.dataType == PipelineDataType::INDEX)
				{
					_cmdList->SetGraphicsRootShaderResourceView(RootParamIndex::indices, _bufferMan->GetResourceHandle(object.mesh.indexBuffer));
				}
				else if (binding.dataType == PipelineDataType::DIFFUSE)
				{
					_cmdList->SetGraphicsRootDescriptorTable(RootParamIndex::diffuse, _textureMan->GetDescriptorHandle(object.surfaceProperty.diffuseTexture));
				}
				else if (binding.dataType == PipelineDataType::SPECULAR)
				{
					_cmdList->SetGraphicsRootDescriptorTable(RootParamIndex::specular, _textureMan->GetDescriptorHandle(object.surfaceProperty.specularTexture));
				}
				else if (binding.dataType == PipelineDataType::SAMPLER)
				{
					continue; // Only static sampler
				}
			}

			const u32 elementCount = _bufferMan->GetElemCount(object.mesh.indexBuffer);

			_cmdList->DrawInstanced(elementCount, 1, 0, 0);
		}
	}
	void Renderer::Present()
	{
		TransitionResource(
			_backbuffers[_currentBackbuffer].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		);

		ExecuteCommandList();
		_swapchain->Present(0, 0);
		FlushCommandQueue();
	}
	void Renderer::ExecuteCommandList()
	{
		_cmdList->Close() >> hrVerify;
		ID3D12CommandList* tmp = _cmdList.Get();
		_cmdQ->ExecuteCommandLists(1, &tmp);
	}

	void Renderer::FlushCommandQueue()
	{
		++_currentFenceValue;
		_cmdQ->Signal(_fence.Get(), _currentFenceValue) >> hrVerify;

		if (_fence->GetCompletedValue() < _currentFenceValue)
		{
			HANDLE eventHandle = CreateEventEx(nullptr, 0, 0, EVENT_ALL_ACCESS);
			_fence->SetEventOnCompletion(_currentFenceValue, eventHandle) >> hrVerify;
			if (eventHandle)
			{
				WaitForSingleObject(eventHandle, INFINITE);
				CloseHandle(eventHandle);
			}
		}

		_heapMan->ResetOffset();
	}
	void Renderer::ResetCommandMemory()
	{
		_cmdAllo->Reset() >> hrVerify;
		_cmdList->Reset(_cmdAllo.Get(), nullptr) >> hrVerify; // nullptr is initial state, no initial state
	}
	void Renderer::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState, D3D12_RESOURCE_STATES newState)
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = resource;
		barrier.Transition.StateBefore = currentState;
		barrier.Transition.StateAfter = newState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		_cmdList->ResourceBarrier(1, &barrier);
	}
	Renderer::~Renderer()
	{
		delete _bufferMan;
		delete _heapMan;
		delete _textureMan;
		delete _samplerMan;
		delete _camera;
		delete _currentPass;
	}
}
