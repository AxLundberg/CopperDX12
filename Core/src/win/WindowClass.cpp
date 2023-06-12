#include "WindowClass.h" 
#include "IWindow.h"

namespace CPR::WIN
{
	LRESULT IWindowClass::ForwardMessage(IWindow* pWnd, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		return pWnd->HandleMessage(hWnd, msg, wParam, lParam);
	}

	WindowClass::WindowClass(const std::wstring& className)
		:
		m_hInstance{ GetModuleHandle(nullptr) }
	{
		const WNDCLASSEXW wc{
			.cbSize = sizeof(wc),
			.style = CS_OWNDC,
			.lpfnWndProc = &WindowClass::HandleMessageSetup,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = m_hInstance,
			.hIcon = nullptr,
			.hCursor = nullptr,
			.hbrBackground = nullptr,
			.lpszMenuName = nullptr,
			.lpszClassName = className.c_str(),
			.hIconSm = nullptr,
		};
		m_atom = RegisterClassExW(&wc);
	}
	ATOM WindowClass::GetAtom() const
	{
		return m_atom;
	}
	HINSTANCE WindowClass::GetInstance() const
	{
		return m_hInstance;
	}
	WindowClass::~WindowClass()
	{
		UnregisterClass(MAKEINTATOM(m_atom), m_hInstance);
	}
	LRESULT WindowClass::HandleMessageSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// use create parameter passed in from CreateWindowExW() to store window class pointer at WinAPI side 
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data 
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			IWindow* const pWnd = static_cast<IWindow*>(pCreate->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window instance 
			SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			// set message proc to normal (non-setup) handler now that setup is finished 
			SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowClass::HandleMessageThunk));
			// forward message to window instance handler 
			return ForwardMessage(pWnd, hWnd, msg, wParam, lParam);
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler 
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	LRESULT WindowClass::HandleMessageThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// retrieve ptr to window instance 
		IWindow* const pWnd = reinterpret_cast<IWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
		// forward message to window instance handler 
		return ForwardMessage(pWnd, hWnd, msg, wParam, lParam);
	}
}
