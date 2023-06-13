#pragma once
#include <deque>
#include <future>
#include <functional>

namespace CPR::CCR
{
	class GenericTaskQueue
	{
		using Task = std::move_only_function<void()>;
	public:
		template <class F>
		auto Push(F&& function)
		{
			using T = std::invoke_result_t<F>;
			std::packaged_task<T()> pkg{ std::forward<F>(function) };
			auto future = pkg.get_future();
			PushWrappedTask([pkg = std::move(pkg)]() mutable { pkg(); });
			return future;
		}
		void PopExecute();

	private:
		void PushWrappedTask(Task task);
	private:
		std::mutex mtx;
		std::deque<Task> tasks;
	};
}