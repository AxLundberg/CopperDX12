#include <Core/src/log/Log.h>

#include "../cmn/GraphicsError.h"
#include "TextureManager.h"

namespace CPR::GFX::D12
{
	TextureManager::TextureManager(ComPtr<ID3D12Device5> const& device, HeapManager* heapManager)
		: _device(device), _heapMan(heapManager)
	{}

	TextureManager::~TextureManager()
	{
		for (ID3D12Resource* texture : _committed)
			texture->Release();
	}

	ResourceIndex TextureManager::AddTexture(void* textureData, const TextureInfo& textureInfo)
	{
		if (!(_committed.size() < _textureDescHandle.size))
			throw std::runtime_error("Descriptor heap full, AddTexture()");

		auto cCount = textureInfo.format.componentCount;
		auto cSize = textureInfo.format.componentSize;
		auto cType = textureInfo.format.componentType;

		DXGI_FORMAT format;
		if (cCount == TexelComponentCount::SINGLE && cSize == TexelComponentSize::DWORD)
		{
			format = cType == TexelComponentType::DEPTH ?
				DXGI_FORMAT_D32_FLOAT :
				DXGI_FORMAT_R32_FLOAT;
		}
		else if (cCount == TexelComponentCount::QUAD && cSize == TexelComponentSize::DWORD)
		{
			format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
		else if (cCount == TexelComponentCount::QUAD && cSize == TexelComponentSize::BYTE)
		{
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
		else
		{
			throw std::runtime_error("Invalid format, AddTexture()");
		}

		auto flags = textureInfo.bindingFlags;
		D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_NONE;
		if (flags & TextureBinding::RENDER_TARGET) resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		else if (flags & TextureBinding::DEPTH_STENCIL) resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		else if (flags & TextureBinding::UNORDERED_ACCESS) resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		ID3D12Resource* texture = CreateTexture2DResource(
			textureInfo.baseTextureWidth,
			textureInfo.baseTextureHeight,
			textureInfo.mipLevels,
			format,
			resourceFlags
		);

		ResourceIndex idx = _committed.size();
		_committed.push_back(texture);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = _textureDescHandle.cpuHandle;
		handle.ptr += idx * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		_device->CreateShaderResourceView(texture, nullptr, handle);

		_heapMan->Upload(textureData, texture, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT, textureInfo.mipLevels);

		return idx;
	}

	ID3D12Resource* TextureManager::CreateTexture2DResource(u64 width, u64 height, u32 mipLevels,
		DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, D3D12_CLEAR_VALUE* clearColour)
	{
		D3D12_HEAP_PROPERTIES heapProperties = {};
		ZeroMemory(&heapProperties, sizeof(heapProperties));
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		desc.Width = static_cast<u32>(width);
		desc.Height = static_cast<u32>(height);
		desc.DepthOrArraySize = 1;
		desc.MipLevels = static_cast<u16>(mipLevels);
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = flags;

		ID3D12Resource* toReturn;
		_device->CreateCommittedResource(&heapProperties,
			D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &desc,
			D3D12_RESOURCE_STATE_COMMON, clearColour, IID_PPV_ARGS(&toReturn)) >> hrVerify;

		return toReturn;
	}

	ID3D12Resource* TextureManager::GetResource(ResourceIndex index)
	{
		return _committed[index];
	}

	D3D12_GPU_VIRTUAL_ADDRESS TextureManager::GetResourceHandle(ResourceIndex index)
	{
		return _committed[index]->GetGPUVirtualAddress();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetDescriptorHandle(ResourceIndex idx)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = _textureDescHandle.gpuHandle;
		handle.ptr += idx * _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		return handle;
	}

	void TextureManager::ReserveHeapSpace(ID3D12DescriptorHeap* descHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 nrOfDescriptors, u32 prevReserved)
	{
		_textureDescHandle.cpuHandle = descHeap->GetCPUDescriptorHandleForHeapStart();
		_textureDescHandle.gpuHandle = descHeap->GetGPUDescriptorHandleForHeapStart();
		_textureDescHandle.cpuHandle.ptr += static_cast<u64>(prevReserved) * _device->GetDescriptorHandleIncrementSize(type);
		_textureDescHandle.gpuHandle.ptr += static_cast<u64>(prevReserved) * _device->GetDescriptorHandleIncrementSize(type);
		_textureDescHandle.size = nrOfDescriptors;
	}
}