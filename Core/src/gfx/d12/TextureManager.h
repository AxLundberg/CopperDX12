#pragma once
#include <vector>

#include "cmn/D12Headers.h"
#include "HeapManager.h"
#include "DescriptorHandle.h"

namespace CPR::GFX::D12
{
	enum class TexelComponentCount
	{
		SINGLE,
		QUAD
	};

	enum class TexelComponentSize
	{
		BYTE,
		WORD
	};

	enum class TexelComponentType
	{
		FLOAT,
		UNORM,
		DEPTH,
	};

	struct FormatInfo
	{
		TexelComponentCount componentCount = TexelComponentCount::QUAD;
		TexelComponentSize componentSize = TexelComponentSize::BYTE;
		TexelComponentType componentType = TexelComponentType::UNORM;
	};

	enum TextureBinding
	{
		NONE = 0,
		SHADER_RESOURCE = 1,
		UNORDERED_ACCESS = 2,
		RENDER_TARGET = 4,
		DEPTH_STENCIL = 8
	};

	struct TextureInfo
	{
		unsigned int mipLevels = 1;
		unsigned int baseTextureWidth = 0;
		unsigned int baseTextureHeight = 0;
		FormatInfo format;
		unsigned int bindingFlags = TextureBinding::NONE;
	};

	class TextureManager
	{
	public:
		TextureManager(ComPtr<ID3D12Device5> const& device, HeapManager*);
		~TextureManager();

		ResourceIndex AddTexture(void* textureData, const TextureInfo& textureInfo);
		ID3D12Resource* GetResource(ResourceIndex idx);
		D3D12_GPU_VIRTUAL_ADDRESS GetResourceHandle(ResourceIndex idx);
		D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorHandle(ResourceIndex idx);
		ID3D12Resource* CreateTexture2DResource(u64 width, u64 height, u32 mipLevels = 1,
			DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE, D3D12_CLEAR_VALUE* clearColour = nullptr);
		void ReserveHeapSpace(ID3D12DescriptorHeap* descHeap, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 nrOfDescriptors, u32 prevReserved = 0);
	private:
		ComPtr<ID3D12Device5> _device;
		HeapManager* _heapMan;
		DescriptorHandle _textureDescHandle = {};
		std::vector<ID3D12Resource*> _committed;
	};
}
