#pragma once

#include <thread>
#include <functional>
#include <semaphore>

namespace Spikey {

	class RenderThread {
	public:
		RenderThread();
		RenderThread(const RenderThread& other) = delete;
		~RenderThread() = default;

		void Join() { m_Thread.join(); }
		void Terminate();

		using TaskDelegate = std::function<void()>;
		void PushTask(TaskDelegate&& delegate);

		void Wait();
		void Process();

		std::thread::id GetID() const { return m_Thread.get_id(); }

	private:
		std::thread m_Thread;
		std::vector<TaskDelegate> m_Queue;
		std::vector<TaskDelegate> m_RTQueue;

		std::binary_semaphore m_BlockSemaphore{ 0 };
		std::binary_semaphore m_WaitSemaphore{ 1 };

		bool m_ShouldTerminate = false;
	};
}