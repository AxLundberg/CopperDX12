#pragma once
#include <Core/src/win/IWindow.h>

#include "base/IDev.h"
#include "cmn/D11Headers.h"


// TODO:	protected data members
//			injections instead of embedded make_shared

namespace CPR::GFX::D11
{
	class IDev : public GFX::IDev
	{
	public:
		struct IocParams
		{
			HWND hWnd;
		};
	public:
		virtual Microsoft::WRL::ComPtr<ID3D11Device> GetD3D11Device() = 0;
		virtual Microsoft::WRL::ComPtr<IDXGISwapChain> GetD3D11SwapChain() = 0;
		virtual Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetD3D11DeviceContext() = 0;
	};

	class Device : public IDev
	{
	public:
		Device(HWND windowHandle);
		~Device();
		Microsoft::WRL::ComPtr<ID3D11Device> GetD3D11Device() override;
		Microsoft::WRL::ComPtr<IDXGISwapChain> GetD3D11SwapChain() override;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetD3D11DeviceContext() override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> device_;
		Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext_;
	};
}