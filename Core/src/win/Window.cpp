#include "Window.h"
#include "Util.h"
#include "Exception.h"
#include "Core/src/log/Log.h"
#include "Core/src/utl/String.h"

#include <format>

namespace CPR::WIN
{
	Window::Window(std::shared_ptr<IWindowClass> pWindowClass, std::shared_ptr<IKeyboardSink> pKeySink,
		std::wstring title, SPA::DimensionsI clientAreaSize, std::optional<SPA::Vec2I> position)
		:
		m_pWindowClass{ std::move(pWindowClass) },
		m_keySink{ std::move(pKeySink) },
		m_kernelThread{ &Window::MessageKernel, this }
	{
		auto future = m_tasks.Push([=, this] {
			const DWORD styles = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
			const DWORD exStyles = 0;
			const auto windowDim = ClientToWindowDimensions(clientAreaSize, styles);
			const auto hModule = GetModuleHandleW(nullptr);
			if (!hModule)
			{
				cprlog.Error().Hr();
				throw WindowException{ "Failed to get module handle" };
			}
			m_hWnd = CreateWindowExW(
				exStyles,
				MAKEINTATOM(m_pWindowClass->GetAtom()),
				title.c_str(),
				styles,
				position.transform([](auto v) {return v.x; }).value_or(CW_USEDEFAULT),
				position.transform([](auto v) {return v.y; }).value_or(CW_USEDEFAULT),
				windowDim.width, windowDim.height,
				nullptr, nullptr, hModule,
				this
			);
			if (!m_hWnd)
			{
				cprlog.Error(L"Failed to create window").Hr();
				throw WindowException{ "Failed to create window" };
			}
		});
		m_startSignal.release();
		future.get();
	}

	HWND Window::GetHandle() const
	{
		return m_hWnd;
	}

	bool Window::IsClosing() const
	{
		return m_closing;
	}

	std::future<void> Window::SetTitle(std::wstring title)
	{
		return Dispatch([=, this] {
			if (!SetWindowTextW(m_hWnd, title.c_str()))
			{
				cprlog.Warn().Hr();
			}
		});
	}

	Window::~Window()
	{
		Dispatch([this] {
			if (!DestroyWindow(m_hWnd))
			{
				cprlog.Warn(L"Failed to destroy window").Hr();
			}
		});
		m_kernelThread.join();
	}

	LRESULT Window::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		try
		{
			switch (msg)
			{
			case WM_DESTROY:
				m_hWnd = nullptr;
				PostQuitMessage(0);
				return 0;
			case WM_CLOSE:
				m_closing = true;
				return 0;
			case CUSTOM_TASK_MESSAGE_ID:
				m_tasks.PopExecute();
				return 0;
			case WM_KEYDOWN:
				m_keySink->PutEvent(KeyEvent{ .type = KeyEvent::Type::Press, .code = (uint8_t)wParam });
				break;
			case WM_KEYUP:
				m_keySink->PutEvent(KeyEvent{ .type = KeyEvent::Type::Release, .code = (uint8_t)wParam });
				break;
			default:
				break;
			}
		}
		catch (const std::exception& e)
		{
			cprlog.Error(std::format(
				L"Uncaught exception in Windows message handler: {}",
				UTL::ToWide(e.what())
			));
		}
		catch (...)
		{
			cprlog.Error(L"Uncaught anonymous exception in Windows message handler");
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Window::NotifyTaskDispatch() const
	{
		if (!PostMessageW(m_hWnd, CUSTOM_TASK_MESSAGE_ID, 0, 0))
		{
			cprlog.Error().Hr();
			throw WindowException{ "Failed to post task notification message" };
		}
	}

	void Window::MessageKernel() noexcept
	{
		m_startSignal.acquire();
		m_tasks.PopExecute();

		MSG msg{};
		while (GetMessage(&msg, m_hWnd, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
}
