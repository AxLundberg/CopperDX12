#include "../../cmn/GraphicsError.h"
#include "DescriptorHeap.h"

namespace CPR::GFX::D12
{
	DescriptorHeap::DescriptorHeap(IDevice* device, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 nrOfDescriptors, bool shaderVisible)
		: 
		_heapType(type),
		_maxSize(nrOfDescriptors)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC desc = 
		{
			.Type = type,
			.NumDescriptors = nrOfDescriptors,
			.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};

		auto d = device->AsD3D12Device();
		d->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descHeap)) >> hrVerify;
	}
	
	DescriptorRange DescriptorHeap::GetRange(std::optional<u64> start, std::optional<u32> rangeSize) const
	{
		auto startIndex = start.value_or(0);
		auto range = rangeSize.value_or(_maxSize - startIndex);
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_start(_descHeap->GetCPUDescriptorHandleForHeapStart().ptr + startIndex * _descriptorSize);
		D3D12_GPU_DESCRIPTOR_HANDLE gpu_start{};
		if (_shaderVisible)
			gpu_start = D3D12_GPU_DESCRIPTOR_HANDLE(_descHeap->GetGPUDescriptorHandleForHeapStart().ptr + startIndex * _descriptorSize);

		return DescriptorRange{
			._heapType = _heapType,
			._cpuHandle = cpu_start,
			._gpuHandle = gpu_start,
			._descriptorSize = _descriptorSize,
			._indexFromStart = startIndex,
			._nrOfDescriptors = range
		};
	}

	void DescriptorHeap::ReserveHeapSpace(u32 nrOfDescriptors, u32 prevReserved)
	{

	}
}