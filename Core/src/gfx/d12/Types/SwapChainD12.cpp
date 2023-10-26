#pragma once
#include "../../Exception.h"
#include "SwapChainD12.h"
#include "Core/src/log/Log.h"

#include <optional>


namespace CPR::GFX::D12
{
	SwapChainD12::SwapChainD12(HWND window, std::optional<u32> nrOfBuffers)
		:
		_window(window),
		_nrOfBuffers(nrOfBuffers.value_or(2))
	{
		if (nrOfBuffers.value() < 2){
			cprlog.Error().Hr();
			throw CPR::GFX::GraphicsException{ "Minimum 2 buffers required for SwapChainD12" };
		}
	}
	void SwapChainD12::SetClearColor(const std::array<float, 4>& clear_color)
	{
	}
	void SwapChainD12::AsD3D12SwapChain(ComPtr<ID3D12CommandQueue> cmdQueue)
	{
#ifndef _DEBUG
		const bool debug_on = true;
#else
		const bool debug_on = false;
#endif // !DEBUG

		ComPtr<IDXGIFactory6> factory;
		u32 factoryFlags = debug_on ? DXGI_CREATE_FACTORY_DEBUG : 0;
		CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&factory)) >> hrVerify;

		const DXGI_SWAP_CHAIN_DESC1 desc = {
			.Width = 0u,
			.Height = 0u,
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = FALSE,
			.SampleDesc = {
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = _nrOfBuffers,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = 0,
		};
		ComPtr<IDXGISwapChain1> swapChain1;
		factory->CreateSwapChainForHwnd(
			cmdQueue.Get(),
			_window,
			&desc,
			nullptr,
			nullptr,
			&swapChain1) >> hrVerify;
		swapChain1.As(&_swapChain) >> hrVerify;
	}
	HWND SwapChainD12::GetHWND()
	{
		return _window;
	}
}
