#include "DeviceD12.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D12
{
	Device::Device()
	{
		D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device_)) >> hrVerify;
	}

	Device::~Device() = default;

	ComPtr<ID3D12Device5> Device::AsD3D12Device()
	{
		return device_;
	}
}
