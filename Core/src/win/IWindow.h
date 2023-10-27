#pragma once 
#include <string>
#include <future>
#include <optional>
#include <Core/src/spa/Vec2.h>
#include <Core/src/spa/Dimensions.h>

#include "CopperWin.h"
#include "Input.h"

namespace CPR::WIN
{
	class IWindow
	{
		// allow WindowClasses access to the message handling function 
		friend class IWindowClass;
	public:
		// types
		struct IocParams
		{
			std::shared_ptr<IWindowClass> pClass;
			std::shared_ptr<IKeyboardSink> pKeySink;
			std::optional<std::wstring> name;
			std::optional<SPA::DimensionsI> size;
			std::optional<SPA::Vec2I> position;
		};
		virtual ~IWindow() = default;
		virtual HWND GetHandle() const = 0;
		virtual bool IsClosing() const = 0;
		virtual std::future<void> SetTitle(std::wstring title) = 0;
	protected:
		virtual LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept = 0;
	};
}
