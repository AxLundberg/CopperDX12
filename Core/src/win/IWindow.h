#pragma once 
#include "CopperWin.h"

namespace CPR::WIN
{
	class IWindow
	{
		// allow WindowClasses access to the message handling function 
		friend class IWindowClass;
	public:
		virtual ~IWindow() = default;
	protected:
		virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept = 0;
	};
}
