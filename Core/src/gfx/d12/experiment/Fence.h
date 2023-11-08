#pragma once
#include <memory>

#include "../../cmn/TypeDefs.h"
#include "../cmn/D12Headers.h"
#include "../DeviceD12.h"

namespace CPR::GFX::D12
{
	class IQueue;

	class IFence
	{
	public:
		virtual ~IFence() = default;
		virtual bool Signal(IQueue&, u32 value) = 0;
		virtual bool GpuWait(IQueue&) = 0;
		virtual bool CpuWait() = 0;
	private:

	};

	class Fence : public IFence
	{
	public:
		Fence(std::shared_ptr<IDevice>, u32 value = 0, D3D12_FENCE_FLAGS = D3D12_FENCE_FLAG_NONE);

		bool Signal(IQueue&, u32 value) override;
		bool GpuWait(IQueue&) override;
		bool CpuWait() override;
	private:
		ComPtr<ID3D12Fence> _fence;
		HANDLE _event = nullptr;
		u32 _waitForValue = 0;
	};
}