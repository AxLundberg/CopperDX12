#pragma once
#include <optional>

#include "../cmn/D12Headers.h"
#include "../../IDevice.h"

namespace CPR::GFX::D12
{
	struct DescriptorRange
	{
		D3D12_DESCRIPTOR_HEAP_TYPE _heapType;
		D3D12_CPU_DESCRIPTOR_HANDLE _cpuHandle{};
		std::optional<D3D12_GPU_DESCRIPTOR_HANDLE> _gpuHandle{};
		u32 _descriptorSize{ 0 };
		u64 _indexFromStart{ 0 };
		u32 _nrOfDescriptors{ 0 };
	};
	class DescriptorHeap
	{
	public:
		DescriptorHeap(IDevice* device, D3D12_DESCRIPTOR_HEAP_TYPE heap_type, u32 max_descriptors = 1, bool shaderVisible = false);

		// Get a ranged chunk
		DescriptorRange GetRange(std::optional<u64> index_start = std::nullopt, std::optional<u32> num_descriptors = std::nullopt) const;

		u32 maxSize() const { return _maxSize; }
		bool ShaderVisible() const { return _shaderVisible; }

		operator ID3D12DescriptorHeap*() const { return _descHeap.Get(); }
	private:
		ComPtr<ID3D12DescriptorHeap> _descHeap;
		D3D12_DESCRIPTOR_HEAP_TYPE _heapType;
		u32 _maxSize{ 0 };
		u8 _descriptorSize{ 0 };
		bool _shaderVisible{ false };
	};

}