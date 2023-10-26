#pragma once
#include <vector>

#include "cmn/D12Headers.h"
#include "experiment/ResourceDesc.h"

namespace CPR::GFX::D12
{
	struct ResourceJob
	{
		u32 firstIdx;
		u32 nrOfResources;

	};
	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();
		void SubmitResource(ComPtr<ID3D12Resource>);
		void SubmitResources(std::vector<ComPtr<ID3D12Resource>>);
		void CreateTextureResource(void* textureData, TextureDesc);
		void TransitionResource(u32 idx);
	private:
		u32 _frameGap;
		std::vector<ComPtr<ID3D12Resource>> _resources;
	};
}