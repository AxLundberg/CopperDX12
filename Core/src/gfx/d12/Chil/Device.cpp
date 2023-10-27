#include "Device.h"
#include <Core/src/gfx/cmn/GraphicsError.h>
#include <Core/src/log/Log.h>

namespace CPR::GFX::D12
{
	using Microsoft::WRL::ComPtr;

	Device::Device()
	{
		// make an env namespace / header for build switch stuff like this
#ifdef NDEBUG
		constexpr bool is_debug = false;
#else
		constexpr bool is_debug = true;
#endif
		// enable the software debug layer for d3d12
		if (is_debug) {
			ComPtr<ID3D12Debug1> debugController;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) >> hrVerify;
			debugController->EnableDebugLayer();
			debugController->SetEnableGPUBasedValidation(true);
		}
		CreateDXGIFactory2(is_debug ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&pDxgiFactory_)) >> hrVerify;
		D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice_)) >> hrVerify;
	}

	Device::~Device() = default;

	ComPtr<ID3D12Device2> Device::GetD3D12DeviceInterface()
	{
		return pDevice_;
	}

	ComPtr<IDXGIFactory4> Device::GetDXGIFactoryInterface()
	{
		return pDxgiFactory_;
	}
}
