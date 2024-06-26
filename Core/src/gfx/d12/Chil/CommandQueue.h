#pragma once
#include "Device.h"
#include <Core/src/win/CopperWin.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <deque>
#include "AllocatorListPair.h"
#include "ICommandQueue.h"

namespace CPR::GFX::D12
{
	class CommandQueue : public ICommandQueue
	{
	public:
		CommandQueue(std::shared_ptr<IDevice> pDevice,
			D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT);
		AllocatorListPair GetAllocatorListPair() override;
		void ExecuteCommandList(AllocatorListPair commandListPair) override;
		uint64_t ExecuteCommandListWithFence(AllocatorListPair commandListPair) override;
		uint64_t SignalFence() override;
		uint64_t SignalFrameFence() override;
		bool FenceHasReached(uint64_t fenceValue) const override;
		void WaitForFenceValue(uint64_t fenceValue) const override;
		// get the value to be signalled on the fence when current frame completes
		uint64_t GetFrameFenceValue() const override;
		// get actual value signalled on the fence (via command list completion etc.)
		uint64_t GetSignalledFenceValue() const override;
		void Flush() override;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const override;
	protected:
		virtual Microsoft::WRL::ComPtr<ID3D12CommandAllocator> MakeCommandAllocator();
		virtual Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList2> MakeCommandList(
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator);
	private:
		// types
		struct CommandAllocatorEntry_
		{
			uint64_t fenceValue;
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
		};
		// data
		static constexpr uint64_t maxFencesPerFrame_ = 100;
		D3D12_COMMAND_LIST_TYPE commandListType_;
		std::shared_ptr<IDevice> pDevice_;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue_;
		Microsoft::WRL::ComPtr<ID3D12Fence> pFence_;
		std::atomic<uint64_t> currentFenceValue_ = 0;
		std::atomic<uint64_t> frameFenceValue_ = maxFencesPerFrame_;
		std::deque<CommandAllocatorEntry_> commandAllocatorQueue_;
		std::vector<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>> commandListPool_;
		std::mutex mutex_;
	};
}