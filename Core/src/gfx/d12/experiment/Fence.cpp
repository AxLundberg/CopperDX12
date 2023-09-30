#include "Fence.h"
#include "Queue.h"
#include "../../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	Fence::Fence(IDevice& device, u32 value, D3D12_FENCE_FLAGS flags)
	{
		auto d = device.AsD3D12Device();
		d->CreateFence(value, flags, IID_PPV_ARGS(&_fence)) >> hrVerify;

		_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}
	bool Fence::Signal(IQueue& queue, u32 value)
	{
		_waitForValue = value;
		HRESULT hr = queue.GetD12_Queue()->Signal(_fence.Get(), value);
		return SUCCEEDED(hr);
	}
	bool Fence::GpuWait(IQueue& queue)
	{
		HRESULT hr = queue.GetD12_Queue()->Wait(_fence.Get(), _waitForValue);
		return SUCCEEDED(hr);
	}
	bool Fence::CpuWait()
	{
		if (_fence->GetCompletedValue() < _waitForValue)
		{
			// Raise an event when fence reaches the value
			HRESULT hr = _fence->SetEventOnCompletion(_waitForValue, _event);
			if (FAILED(hr))
				return false;

			// CPU block until event is done
			WaitForSingleObject(_event, INFINITE);
		}

		return true;
	}
}
