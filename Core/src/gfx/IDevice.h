#pragma once
#include "d12/cmn/D12Headers.h"
#include "cmn/GraphicsError.h"

namespace CPR::GFX
{
    
    class IDevice {
            // The common interface for all devices
            // Specific devices (D3D12, Vulkan, etc.) should implement this
    public:
        virtual ~IDevice() = default;
        virtual Microsoft::WRL::ComPtr<ID3D12Device5> AsD3D12Device() { return nullptr; }
        //virtual Microsoft::WRL::ComPtr<ID3D11Device> AsD3D11Device() { return nullptr; }
        
    };
    


    //class D3D11Device : public IDevice {
    //public:
    //    D3D11Device()
    //    {
    //        // Initialize mDevice
    //    }
    //    Microsoft::WRL::ComPtr<ID3D11Device> AsD3D1Device()
    //    {
    //        return mDevice;
    //    }
    //private:
    //    Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
    //};
}