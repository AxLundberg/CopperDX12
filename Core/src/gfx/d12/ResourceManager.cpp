#include "ResourceManager.h"

namespace CPR::GFX::D12
{
	ResourceManager::ResourceManager()
	{
	}
	ResourceManager::~ResourceManager()
	{
	}
	void ResourceManager::SubmitResource(ComPtr<ID3D12Resource> r)
	{
		_resources.push_back(r);
	}
	void ResourceManager::SubmitResources(std::vector<ComPtr<ID3D12Resource>> v)
	{
		for (auto& r : v) {
			SubmitResource(r);
		}
	}
	void ResourceManager::CreateTextureResource(void* textureData, TextureDesc td)
	{

	}
	void ResourceManager::TransitionResource(u32 idx)
	{
	}
}