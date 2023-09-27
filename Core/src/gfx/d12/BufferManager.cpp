#include "../cmn/GraphicsError.h"
#include "BufferManager.h"

namespace CPR::GFX::D12
{
	BufferManager::BufferManager(ComPtr<ID3D12Device5> const& device, HeapManager* heapManager)
		: _device(device), _heapMan(heapManager)
	{
	}
	BufferManager::~BufferManager()
	{
	}
	ResourceIndex BufferManager::SubmitBuffer(void* data, u32 elemSize, u32 elemCount, PerFrameUsage rwPattern, BufferBinding, u32 bindFlags)
	{
		auto idx = _committed.size();

		D3D12_RESOURCE_STATES state = rwPattern == PerFrameUsage::STATIC ?
			D3D12_RESOURCE_STATE_COMMON :
			D3D12_RESOURCE_STATE_GENERIC_READ;

		D3D12_HEAP_PROPERTIES properties = {};
		properties.Type = rwPattern == PerFrameUsage::STATIC ?
			D3D12_HEAP_TYPE_DEFAULT :
			D3D12_HEAP_TYPE_UPLOAD;

		properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		properties.CreationNodeMask = 0;
		properties.VisibleNodeMask = 0;

		ID3D12Resource* resource = {};
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = static_cast<u64>(elemSize) * static_cast<u64>(elemCount);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		_device->CreateCommittedResource(&properties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES,
			&desc, state, nullptr, IID_PPV_ARGS(&resource)) >> hrVerify;

		Buffer newBuf = {};
		newBuf.resource = resource;
		newBuf.sizeOfElement = elemSize;
		newBuf.nrOfElements = elemCount;
		_committed.push_back(newBuf);

		if (rwPattern == PerFrameUsage::STATIC)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Buffer.FirstElement = 0;
			desc.Buffer.NumElements = elemCount;
			desc.Buffer.StructureByteStride = elemSize;
			D3D12_CPU_DESCRIPTOR_HANDLE handle = _descHandle.cpuHandle;
			handle.ptr += idx * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			_device->CreateShaderResourceView(resource, &desc, handle);
			_heapMan->Upload(data, resource, 4u, 0); // AXEL
		}
		else
		{
			UpdateBuffer(idx, data);
		}

		return idx;
	}
	void BufferManager::UpdateBuffer(ResourceIndex bufferIndex, void* data)
	{
		if (data == nullptr) // AXEL
		{
			int bogus = 0;
			data = static_cast<void*>(&bogus);
		}
		auto& buffer = _committed[bufferIndex];
		u8* subresource = {};
		buffer.resource->Map(0u, nullptr, reinterpret_cast<void**>(&subresource)) >> hrVerify;

		memcpy(reinterpret_cast<void*>(subresource), reinterpret_cast<void*>(data), buffer.sizeOfElement);
		buffer.resource->Unmap(0u, nullptr);
	}
	u32 BufferManager::GetElemSize(ResourceIndex bufferIndex)
	{
		return _committed[bufferIndex].sizeOfElement;
	}
	u32 BufferManager::GetElemCount(ResourceIndex bufferIndex)
	{
		return _committed[bufferIndex].nrOfElements;
	}
	D3D12_GPU_VIRTUAL_ADDRESS BufferManager::GetResourceHandle(ResourceIndex bufferIndex)
	{
		return _committed[bufferIndex].resource->GetGPUVirtualAddress();
	}
	D3D12_GPU_DESCRIPTOR_HANDLE BufferManager::GetDescriptorHandle(ResourceIndex bufferIndex)
	{
		auto handle = _descHandle.gpuHandle;
		handle.ptr += bufferIndex * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return handle;
	}
}
