#pragma once
#include "cmn/D12Headers.h"

namespace CPR::GFX::D12
{
	class HeapManager
	{
	private:
		static constexpr u32 HEAP_BYTE_SIZE = 200000000;

	public:
		HeapManager() = default;
		HeapManager(ComPtr<ID3D12Device5> const& device, ComPtr<ID3D12GraphicsCommandList> const& list);
		~HeapManager();

		void InitHeap(D3D12_HEAP_TYPE heapType);
		void PlaceResource();
		void Upload(void* data, ID3D12Resource* targetResource, u32 alignment, u32 subresource);
		void ResetOffset();

	private:
		u32 MemcpyUploadData(u8* mappedPtr, u8* data, u32 uploadOffset, u32 alignment,
			u32 nrOfRows, u64 rowSizeInBytes, u32 rowPitch);
		void UploadData(ID3D12Resource* targetResource, u8* data, u32 alignment, u32 subresource);

	private:
		u32 _heapOffset = 0;
		ComPtr<ID3D12Device5> _device;
		ComPtr<ID3D12Heap> _uploadHeap;
		ComPtr<ID3D12Resource> _uploadBuffer;
		ComPtr<ID3D12GraphicsCommandList> _cmdList;
	};
}