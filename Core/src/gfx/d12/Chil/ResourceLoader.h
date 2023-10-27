#pragma once
#include "Device.h"
#include <future>
#include "ICommandQueue.h"
#include "Texture.h"

namespace CPR::GFX::D12
{
	class IResourceLoader
	{
	public:
		virtual ~IResourceLoader() = default;
		virtual std::future<std::shared_ptr<ITexture>> LoadTexture(std::wstring path) = 0;
	};

	class ResourceLoader : public IResourceLoader
	{
	public:
		ResourceLoader(std::shared_ptr<IDeviceChil> pDevice);
		std::future<std::shared_ptr<ITexture>> LoadTexture(std::wstring path) override;
	private:
		std::shared_ptr<IDeviceChil> pDevice_;
		std::shared_ptr<ICommandQueue> pQueue_;
	};
}
