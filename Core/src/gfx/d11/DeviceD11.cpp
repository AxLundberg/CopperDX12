#include "../cmn/GraphicsError.h"

#include "DeviceD11.h"

namespace CPR::GFX::D11
{
	Device::Device(HWND hWnd)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

		swapChainDesc.BufferDesc.Width = 0;
		swapChainDesc.BufferDesc.Height = 0;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.OutputWindow = hWnd;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
			nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION,
			&swapChainDesc, &swapChain_, &device_, nullptr, &deviceContext_) >> hrVerify;
	}

	Device::~Device() = default;

	ComPtr<ID3D11Device> Device::GetD3D11Device()
	{
		return device_;
	}
	ComPtr<IDXGISwapChain> Device::GetD3D11SwapChain()
	{
		return swapChain_;
	}
	ComPtr<ID3D11DeviceContext> Device::GetD3D11DeviceContext()
	{
		return deviceContext_;
	}
}