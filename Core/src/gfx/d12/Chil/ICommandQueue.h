#pragma once
#include <Core/src/win/CopperWin.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include "AllocatorListPair.h"

namespace CPR::GFX::D12
{
	class ICommandQueue
	{
	public:
		virtual ~ICommandQueue() = default;
		virtual AllocatorListPair GetAllocatorListPair() = 0;
		virtual void ExecuteCommandList(AllocatorListPair commandListPair) = 0;
		virtual uint64_t ExecuteCommandListWithFence(AllocatorListPair commandListPair) = 0;
		virtual uint64_t SignalFence() = 0;
		virtual uint64_t SignalFrameFence() = 0;
		virtual bool FenceHasReached(uint64_t fenceValue) const = 0;
		virtual void WaitForFenceValue(uint64_t fenceValue) const = 0;
		// get the value to be signalled on the fence when current frame completes
		virtual uint64_t GetFrameFenceValue() const = 0;
		// get actual value signalled on the fence (via command list completion etc.)
		virtual uint64_t GetSignalledFenceValue() const = 0;
		virtual void Flush() = 0;
		virtual Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetD3D12CommandQueue() const = 0;
	};
}