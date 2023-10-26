#pragma once
#include <queue>
#include <memory>
#include "../cmn/D12Headers.h"
#include "../../IDevice.h"

namespace CPR::GFX::D12
{
	class ResourcePool
	{
	public:
		ResourcePool(std::shared_ptr<CPR::GFX::IDevice> _device);
		~ResourcePool();

	private:
		std::queue<ComPtr<ID3D12Resource>> availableResources;
	};
}
