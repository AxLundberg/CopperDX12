#pragma once
#include "IWindow.h"
#include "WindowClass.h"
#include <string>
#include <thread>
#include <atomic>
#include <optional>
#include <semaphore>
#include <Core/src/spa/Rect.h>
#include <Core/src/spa/Dimensions.h>
#include <Core/src/ccr/GenericTaskQueue.h>

namespace CPR::WIN
{
	class Window : public IWindow
	{
	public:
		Window(std::shared_ptr<IWindowClass> pWindowClass, std::shared_ptr<IKeyboardSink> pKeySink, 
			std::wstring title, SPA::DimensionsI clientAreaSize, std::optional<SPA::Vec2I> position = {});
		HWND GetHandle() const override;
		bool IsClosing() const override;
		std::future<void> SetTitle(std::wstring title) override;

		~Window() override;
	protected: 
		// functions
		virtual void MessageKernel() noexcept;
		LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept override;
		template <std::invocable F>
		auto Dispatch(F&& f) const
		{
			auto future = m_tasks.Push(std::forward<F>(f));
			NotifyTaskDispatch();
			return future;
		}
		void NotifyTaskDispatch() const;
	protected: 
		// constants & data
		static constexpr UINT CUSTOM_TASK_MESSAGE_ID = WM_USER + 0;
		std::shared_ptr<IWindowClass> m_pWindowClass;
		std::shared_ptr<IKeyboardSink> m_keySink;
		std::binary_semaphore m_startSignal{ 0 };
		mutable CCR::GenericTaskQueue m_tasks;
		std::atomic<bool> m_closing = false;
		std::thread m_kernelThread;
		HWND m_hWnd = nullptr;
	private:
	};
}
