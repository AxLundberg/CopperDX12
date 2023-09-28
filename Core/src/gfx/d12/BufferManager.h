#pragma once
#include <vector>

#include "DescriptorHandle.h"
#include "cmn/D12Headers.h"
#include "HeapManager.h"

namespace CPR::GFX::D12
{
	enum class PerFrameUsage
	{
		STATIC,
		DYNAMIC
	};

	enum BufferBinding
	{
		STRUCTURED_BUFFER = 1,
		CONSTANT_BUFFER = 2
	};

	struct Buffer
	{
		ID3D12Resource* resource;
		u32 sizeOfElement;
		u32 nrOfElements;
	};

	class BufferManager
	{
	public:
		BufferManager(ComPtr<ID3D12Device5> const& device, HeapManager* heapManager);
		~BufferManager();
		ResourceIndex SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage, u32 bindFlags);
		void UpdateBuffer(ResourceIndex, void* data);
		u32 GetElemSize(ResourceIndex);
		u32 GetElemCount(ResourceIndex);

		void ReserveHeapSpace(ID3D12DescriptorHeap* descHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 nrOfDescriptors, u32 prevReserved = 0);

		D3D12_GPU_VIRTUAL_ADDRESS GetResourceHandle(ResourceIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle(ResourceIndex);
	private:
		ComPtr<ID3D12Device5> const _device;
		HeapManager* _heapMan;
		DescriptorHandle _descHandle = {};
		std::vector<Buffer> _committed;
	};
}