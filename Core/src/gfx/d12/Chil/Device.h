#pragma once
#include "../../IDevice.h"
#include <Core/src/win/IWindow.h>
#include <d3d12.h> 
#include <dxgi1_6.h>
#include <wrl/client.h>


// TODO:	protected data members
//			injections instead of embedded make_shared

namespace CPR::GFX::D12
{
	class IDevice : public GFX::IDevice
	{
	public:
		virtual Microsoft::WRL::ComPtr<ID3D12Device2> GetD3D12DeviceInterface() = 0;
		virtual Microsoft::WRL::ComPtr<IDXGIFactory4> GetDXGIFactoryInterface() = 0;
	};

	class Device : public IDevice
	{
	public:
		Device();
		~Device();
		Microsoft::WRL::ComPtr<ID3D12Device2> GetD3D12DeviceInterface() override;
		Microsoft::WRL::ComPtr<IDXGIFactory4> GetDXGIFactoryInterface() override;
	private:
		Microsoft::WRL::ComPtr<ID3D12Device2> pDevice_;
		Microsoft::WRL::ComPtr<IDXGIFactory4> pDxgiFactory_;
	};
}