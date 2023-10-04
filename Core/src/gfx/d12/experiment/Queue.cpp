#include "Queue.h"
#include "Fence.h"
#include "../../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	QueueD12::QueueD12(std::shared_ptr<IDevice> device, std::shared_ptr<IFence> flushFence, D3D12_COMMAND_LIST_TYPE type)
		: 
		_flushFence(std::move(flushFence))
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = type;
		device->AsD3D12Device()->CreateCommandQueue(&desc, IID_PPV_ARGS(&_queue)) >> hrVerify;
	}
	void QueueD12::ExecuteList(IAllocatorAndList* atorAndList)
	{
		auto list = atorAndList->GetList();
		list->Close();
		ID3D12CommandList* tmp = list.Get();
		_queue->ExecuteCommandLists(1, &tmp);
	}
	void QueueD12::SubmitSignal(IFence* fence)
	{
		fence->Signal(*this, _nextFenceValue++);
	}
	void QueueD12::SubmitWait(IFence& fence)
	{
		fence.GpuWait(*this);
	}
	void QueueD12::Flush()
	{
		SubmitSignal(_flushFence.get());
		_flushFence->CpuWait();
	}
	ComPtr<ID3D12CommandQueue> QueueD12::GetD12_Queue()
	{
		return _queue;
	}
}