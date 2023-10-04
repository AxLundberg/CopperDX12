#include "AllocatorAndList.h"
#include "../../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	AllocatorAndList::AllocatorAndList(std::shared_ptr<IDevice> device, D3D12_COMMAND_LIST_TYPE queueType)
	{
		auto d = device->AsD3D12Device();
		d->CreateCommandAllocator(queueType, IID_PPV_ARGS(&_allocator)) >> hrVerify;
		d->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _allocator.Get(), nullptr, IID_PPV_ARGS(&_list)) >> hrVerify;
	}
	ComPtr<ID3D12GraphicsCommandList4> AllocatorAndList::GetList()
	{
		return _list;
	}
	void AllocatorAndList::Reset()
	{
		_allocator->Reset() >> hrVerify;
		_list->Reset(_allocator.Get(), nullptr) >> hrVerify;
	}
	void AllocatorAndList::Close()
	{
		_list->Close() >> hrVerify;
	}
}