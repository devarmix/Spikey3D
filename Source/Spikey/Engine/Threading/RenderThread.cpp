#include <Engine/Threading/RenderThread.h>

namespace Spikey {

	RenderThread::RenderThread() {
		m_Thread = std::thread([this]() {
			PROFILE_THREAD_NAME("Render Thread");

			while (true) {
				m_BlockSemaphore.acquire();

				for (auto& task : m_RTQueue) {
					task();
				}

				// signal end of processing
				m_WaitSemaphore.release();
				if (m_ShouldTerminate) break;
			}
			});
	}

	void RenderThread::PushTask(TaskDelegate&& delegate) {
		m_Queue.push_back(std::forward<TaskDelegate>(delegate));
	}

	void RenderThread::Terminate() {
		PushTask([this]() { m_ShouldTerminate = true; });
		Process();
	}

	void RenderThread::Wait() {
		m_WaitSemaphore.acquire();
	}

	void RenderThread::Process() {
		m_RTQueue = std::move(m_Queue);
		m_Queue.clear();

		m_BlockSemaphore.release();
	}
}