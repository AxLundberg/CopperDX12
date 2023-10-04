#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "AllocatorAndList.h"
#include "Queue.h"
#include "Fence.h"

namespace CPR::GFX::D12
{
	class ISyncCommander
	{
	public:
		virtual ~ISyncCommander() = default;
		virtual void AttachAllocatorAndList(std::shared_ptr<IAllocatorAndList> atorAndList) = 0;
		virtual void AttachQueue(std::shared_ptr<IQueue> queue) = 0;
		virtual void AttachFence(std::shared_ptr<IFence> fence) = 0;
		virtual void RecordCommands(std::function<void()> func) = 0;
		virtual void ExecuteList(u32 idx = 0) = 0;

		virtual std::shared_ptr<IAllocatorAndList> GetAllocatorAndList(u32 idx = 0) = 0;
		virtual ComPtr<ID3D12GraphicsCommandList4> GetList(u32 idx = 0) = 0;
		virtual std::shared_ptr<IQueue> GetQueue(u32 idx = 0) = 0;
		virtual std::shared_ptr<IFence> GetFence(u32 idx = 0) = 0;
	private:
	};
	
	class SyncCommander : public ISyncCommander
	{
	public:
		SyncCommander();
		void AttachAllocatorAndList(std::shared_ptr<IAllocatorAndList> atorAndList) override;
		void AttachQueue(std::shared_ptr<IQueue> queue) override;
		void AttachFence(std::shared_ptr<IFence> fence) override;
		void RecordCommands(std::function<void()> func) override;
		void ExecuteList(u32 idx) override;


		ComPtr<ID3D12GraphicsCommandList4> GetList(u32 idx = 0) override;
		std::shared_ptr<IAllocatorAndList> GetAllocatorAndList(u32 idx = 0) override;
		std::shared_ptr<IQueue> GetQueue(u32 idx = 0) override;
		std::shared_ptr<IFence> GetFence(u32 idx = 0) override;
	private:
		std::vector<std::shared_ptr<IAllocatorAndList>> _lists;
		std::vector<std::shared_ptr<IQueue>> _queues;
		std::vector<std::shared_ptr<IFence>> _fences;
	};
}
