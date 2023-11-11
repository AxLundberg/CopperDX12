#pragma once
#include "../../IRenderPane.h"
#include "CommandQueue.h"
#include "Device.h"
#include <Core/src/win/IWindow.h>
#include <array>
#include "Texture.h"
#include <DirectXMath.h>
#include <optional>

namespace CPR::GFX::D12
{
	class IRenderPane : public GFX::IRenderPane
	{
	public:
		virtual void BeginFrame() = 0;
		virtual AllocatorListPair GetCommandList() = 0;
		virtual void SubmitCommandList(AllocatorListPair commands) = 0;
		virtual uint64_t GetFrameFenceValue() const = 0;
		virtual uint64_t GetSignalledFenceValue() const = 0;
		virtual void EndFrame() = 0;
		virtual void FlushQueues() const = 0;
	};

	class RenderPane : public IRenderPane
	{
	public:
		RenderPane(HWND hWnd, const SPA::DimensionsI& dims, std::shared_ptr<IDevice> pDevice,
			std::shared_ptr<ICommandQueue> pCommandQueue);
		~RenderPane();
		void BeginFrame() override;
		AllocatorListPair GetCommandList() override;
		void SubmitCommandList(AllocatorListPair commands) override;
		uint64_t GetFrameFenceValue() const override;
		uint64_t GetSignalledFenceValue() const override;
		void EndFrame() override;
		void FlushQueues() const override;
	private:
		void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES current, D3D12_RESOURCE_STATES newState);
	private:
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 tc;
		};
		// data
		SPA::DimensionsI dims_;
		std::shared_ptr<IDevice> pDevice_;
		std::shared_ptr<ICommandQueue> pCommandQueue_;
		static constexpr UINT bufferCount_ = 2;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain_;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pRtvDescriptorHeap_;
		UINT rtvDescriptorSize_;
		Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers_[bufferCount_];
		UINT curBackBufferIndex_ = 0;
		uint64_t bufferFenceValues_[bufferCount_]{};
		std::optional<DirectX::XMFLOAT4> clearColor_ = DirectX::XMFLOAT4{ 0.f, 0.f, 0.f, 1.f };
		// depth
		Microsoft::WRL::ComPtr<ID3D12Resource> pDepthBuffer_;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> pDsvDescriptorHeap_;
		// pipey
		D3D12_RECT scissorRect_{};
		D3D12_VIEWPORT viewport_{};
	};
}
