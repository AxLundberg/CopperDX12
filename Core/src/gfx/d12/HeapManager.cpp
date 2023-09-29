#include "HeapManager.h"
#include "../cmn/GraphicsError.h"
#include "../cmn/Helpers.h"

namespace CPR::GFX::D12
{
	CPR::GFX::D12::HeapManager::HeapManager(ComPtr<ID3D12Device5> const& device, ComPtr<ID3D12GraphicsCommandList> const& list)
		: _device(device), _cmdList(list)
	{
		InitHeap(D3D12_HEAP_TYPE_UPLOAD);
		PlaceResource();
	}
	HeapManager::~HeapManager()
	{
	}
	void HeapManager::InitHeap(D3D12_HEAP_TYPE heapType)
	{
		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = heapType;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0;
		heapProperties.VisibleNodeMask = 0;

		D3D12_HEAP_DESC heapDesc = {};
		heapDesc.SizeInBytes = HEAP_BYTE_SIZE;
		heapDesc.Properties = heapProperties;
		heapDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		heapDesc.Flags = D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES | D3D12_HEAP_FLAG_DENY_NON_RT_DS_TEXTURES;

		_device->CreateHeap(&heapDesc, IID_PPV_ARGS(&_uploadHeap)) >> hrVerify;
	}
	void HeapManager::PlaceResource()
	{
		D3D12_RESOURCE_DESC resourceDesc = {};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		resourceDesc.Width = HEAP_BYTE_SIZE;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		_device->CreatePlacedResource(
			_uploadHeap.Get(),
			_heapOffset,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_uploadBuffer)
		) >> hrVerify;
	}
	void HeapManager::Upload(void* data, ID3D12Resource* targetResource, u32 alignment, u32 subresource)
	{
		if (subresource == 0)
		{
			u8* datat = reinterpret_cast<u8*>(data);
			return UploadData(targetResource, datat, alignment, subresource);
		}
		D3D12_RESOURCE_DESC resourceDesc = targetResource->GetDesc();
		u8* tex = reinterpret_cast<u8*>(data);
		u32 currentTextureOffset = 0;
		for (u32 mipLevel = 0; mipLevel < subresource; ++mipLevel)
		{
			UploadData(targetResource, tex + currentTextureOffset, alignment, mipLevel);
			currentTextureOffset += static_cast<u32>((resourceDesc.Width * resourceDesc.Height * 4) / pow(4, mipLevel));
		}

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = targetResource;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		_cmdList->ResourceBarrier(1, &barrier);
	}
	void HeapManager::UploadData(ID3D12Resource* targetResource, u8* data, u32 alignment, u32 subresource)
	{
		D3D12_RANGE empty = { 0u, 0u };
		u8* mappedPtr = nullptr;
		_uploadBuffer->Map(0, &empty, reinterpret_cast<void**>(&mappedPtr)) >> hrVerify;

		D3D12_RESOURCE_DESC resourceDesc = targetResource->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
		u32 nrOfRows = 0u;
		u64 rowSizeInBytes = 0u;
		u64 totalBytes = 0u;
		_device->GetCopyableFootprints(&resourceDesc, subresource, 1u, 0u, &footprint, &nrOfRows, &rowSizeInBytes,
			&totalBytes);

		u32 newOffset = MemcpyUploadData(mappedPtr, data, _heapOffset, alignment, nrOfRows, rowSizeInBytes,
			footprint.Footprint.RowPitch);

		if (resourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) // texture?
		{
			D3D12_TEXTURE_COPY_LOCATION destination = {};
			destination.pResource = targetResource;
			destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			destination.SubresourceIndex = subresource;

			D3D12_TEXTURE_COPY_LOCATION source = {};
			source.pResource = _uploadBuffer.Get();
			source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			source.PlacedFootprint.Offset = AlignAdress(_heapOffset, alignment);
			source.PlacedFootprint.Footprint.Width = footprint.Footprint.Width;
			source.PlacedFootprint.Footprint.Height = footprint.Footprint.Height;
			source.PlacedFootprint.Footprint.Depth = footprint.Footprint.Depth;
			source.PlacedFootprint.Footprint.RowPitch = footprint.Footprint.RowPitch;
			source.PlacedFootprint.Footprint.Format = resourceDesc.Format;

			_cmdList->CopyTextureRegion(&destination, 0u, 0u, 0u, &source, nullptr);
		}
		else
		{
			_cmdList->CopyBufferRegion(targetResource, 0u, _uploadBuffer.Get(),
				AlignAdress(_heapOffset, alignment), resourceDesc.Width);
		}

		_uploadBuffer->Unmap(0u, nullptr);
		_heapOffset = newOffset;
	}
	u32 HeapManager::MemcpyUploadData(u8* mappedPtr, u8* data, u32 uploadOffset, u32 alignment, u32 nrOfRows, u64 rowSizeInBytes, u32 rowPitch)
	{
		u64 sourceOffset = 0;
		u32 destinationOffset = AlignAdress(uploadOffset, alignment);
		for (u32 row = 0; row < nrOfRows; ++row)
		{
			std::memcpy(mappedPtr + destinationOffset, data + sourceOffset, rowSizeInBytes);
			sourceOffset += rowSizeInBytes;
			destinationOffset += rowPitch;
		}

		return destinationOffset;
	}
	void HeapManager::ResetOffset()
	{
		_heapOffset = 0;
	}
}