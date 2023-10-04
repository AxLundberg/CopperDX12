#include "SyncCommander.h"

namespace CPR::GFX::D12
{
	SyncCommander::SyncCommander()
	{}
	void SyncCommander::AttachQueue(std::shared_ptr<IQueue> queue)
	{
		_queues.push_back(std::move(queue));
	}
	void SyncCommander::AttachFence(std::shared_ptr<IFence> fence)
	{
		_fences.push_back(std::move(fence));
	}
	void SyncCommander::RecordCommands(std::function<void()> func)
	{
		func();
	}
	void SyncCommander::ExecuteList(u32 idx)
	{
		_queues[idx]->ExecuteList(_lists[idx].get());
	}
	void SyncCommander::AttachAllocatorAndList(std::shared_ptr<IAllocatorAndList> atorAndList)
	{
		_lists.push_back(std::move(atorAndList));
	}
	std::shared_ptr<IAllocatorAndList> SyncCommander::GetAllocatorAndList(u32 idx)
	{
		return _lists[idx];
	}
	ComPtr<ID3D12GraphicsCommandList4> SyncCommander::GetList(u32 idx)
	{
		return _lists[idx]->GetList();
	}
	std::shared_ptr<IQueue> SyncCommander::GetQueue(u32 idx)
	{
		return _queues[idx];
	}
	std::shared_ptr<IFence> SyncCommander::GetFence(u32 idx)
	{
		return _fences[idx];
	}

}