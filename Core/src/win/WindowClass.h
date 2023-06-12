#pragma once 
#include "CopperWin.h"
#include <string>

namespace CPR::WIN
{
	class IWindowClass
	{
	public:
		virtual ~IWindowClass() = default;
		virtual ATOM GetAtom() const = 0;
		virtual HINSTANCE GetInstance() const = 0;
	protected:
		// allows descendants of IWindowClass access to the IWindow virtual message handler 
		static LRESULT ForwardMessage(class IWindow* pWnd, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	};

	class WindowClass : public IWindowClass
	{
	public:
		WindowClass(const std::wstring& className = L"$Cpr-Default-Window-Class$");
		ATOM GetAtom() const override;
		HINSTANCE GetInstance() const override;
		~WindowClass() override;
	private:
		// functions 
		static LRESULT CALLBACK HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK HandleMessageThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		// data 
		HINSTANCE m_hInstance;
		ATOM m_atom;
	};
}
