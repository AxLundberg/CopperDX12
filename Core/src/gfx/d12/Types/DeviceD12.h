#pragma once
#include "../../IDevice.h"

namespace CPR::GFX::D12
{
    class DeviceD12 : public IDevice {
    public:
        DeviceD12()
        {
            D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_device)) >> hrVerify;
        }
        Microsoft::WRL::ComPtr<ID3D12Device5> AsD3D12Device() override
        {
            return _device;
        }
    private:
        Microsoft::WRL::ComPtr<ID3D12Device5> _device;
    };
}
