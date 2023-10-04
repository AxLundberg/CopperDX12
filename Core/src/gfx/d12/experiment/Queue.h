#pragma once
#include <memory>

#include "../../cmn/TypeDefs.h"
#include "../../IDevice.h"
#include "../cmn/D12Headers.h"
#include "AllocatorAndList.h"

namespace CPR::GFX::D12
{
	class IFence;

	class IQueue
	{
	public:
		virtual ~IQueue() = default;
		virtual void ExecuteList(IAllocatorAndList* list) = 0;
		virtual void SubmitSignal(IFence*) = 0;
		virtual void SubmitWait(IFence&) = 0;
		virtual void Flush() = 0;

		virtual ComPtr<ID3D12CommandQueue> GetD12_Queue() { return nullptr; };
	private:

	};

	class QueueD12 : public IQueue
	{
	public:
		QueueD12(std::shared_ptr<IDevice>, std::shared_ptr<IFence>, D3D12_COMMAND_LIST_TYPE);
		void ExecuteList(IAllocatorAndList* list) override;
		void SubmitSignal(IFence*) override;
		void SubmitWait(IFence&) override;
		void Flush() override;
		ComPtr<ID3D12CommandQueue> GetD12_Queue() override;
	private:
		ComPtr<ID3D12CommandQueue> _queue;
		std::shared_ptr<IFence> _flushFence;
		u32 _nextFenceValue = 1;
	};
}