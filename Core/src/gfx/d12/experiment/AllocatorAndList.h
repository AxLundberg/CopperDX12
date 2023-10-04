#pragma once
#include <memory>
#include <functional>

#include "../../IDevice.h"
#include "../cmn/D12Headers.h"
#include "../Types/QueueTypes.h"


namespace CPR::GFX::D12
{
	class IAllocatorAndList
	{
	public:
		virtual ~IAllocatorAndList() = default;
		virtual void Reset() = 0;
		virtual void Close() = 0;
		virtual ComPtr<ID3D12GraphicsCommandList4> GetList() = 0;
	private:

	};

	class AllocatorAndList : public IAllocatorAndList
	{
	public:
		AllocatorAndList(std::shared_ptr<IDevice>, D3D12_COMMAND_LIST_TYPE = D3D12_COMMAND_LIST_TYPE_DIRECT);
		ComPtr<ID3D12GraphicsCommandList4> GetList() override;
		void Reset() override;
		void Close() override;
	private:
		ComPtr<ID3D12CommandAllocator> _allocator;
		ComPtr<ID3D12GraphicsCommandList4> _list;
	};
}