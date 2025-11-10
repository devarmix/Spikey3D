#pragma once
#include <Engine/Core/Common.h>

namespace Spikey {

	class IndexQueue {
	public:
		IndexQueue() : NextIndex(0) {}

		uint32 Grab() {
			uint32 index = NextIndex;

			if (Queue.size() > 0) {
				index = Queue.back();
				Queue.pop_back();
			}
			else {
				NextIndex++;
			}
			return index;
		}

		void Release(uint32 index) { Queue.push_back(index); }

	private:
		uint32 NextIndex;
		std::vector<uint32> Queue;
	};

	template<typename T>
	void SwapDelete(std::vector<T>& v, uint32_t idx) {
		if (v.size() > 0) {
			v[idx] = std::move(v.back());
		}
		v.pop_back();
	}

	class IRefCounted {
	public:
		virtual ~IRefCounted() = default;

		virtual void AddRef() const = 0;
		virtual void Release() const = 0;

		uint32_t GetRefCount() const { return m_Counter; }
	protected:
		mutable uint32_t m_Counter{ 0 };
	};

	// for derived IRefCounted classes
	template<typename T>
	class TRef {
	public:
		TRef() : m_Ptr(nullptr) {}
		TRef(T* ptr) {
			m_Ptr = ptr;
			if (m_Ptr) {
				m_Ptr->AddRef();
			}
		}

		TRef(const TRef& copy) {
			m_Ptr = copy.m_Ptr;
			if (m_Ptr) {
				m_Ptr->AddRef();
			}
		}

		template<typename CopyT>
		explicit TRef(const TRef<CopyT>& copy) {
			m_Ptr = static_cast<T*>(copy.Get());
			if (m_Ptr) {
				m_Ptr->AddRef();
			}
		}

		template<typename CopyT>
		TRef<CopyT> As() const {
			return TRef<CopyT>(*this);
		}

		TRef(TRef&& move) noexcept {
			m_Ptr = move.m_Ptr;
			move.m_Ptr = nullptr;
		}

		template<typename MoveT>
		explicit TRef(TRef<MoveT>&& move) {
			m_Ptr = static_cast<T*>(move.Get());
			move.m_Ptr = nullptr;
		}

		~TRef() {
			if (m_Ptr) {
				m_Ptr->Release();
			}
		}

		TRef& operator=(T* ptr) {
			T* old = m_Ptr;
			m_Ptr = ptr;

			if (m_Ptr) {
				m_Ptr->AddRef();
			}
			if (old) {
				old->Release();
			}

			return *this;
		}

		TRef& operator=(const TRef& other) {
			T* old = m_Ptr;
			m_Ptr = other.m_Ptr;

			if (m_Ptr) {
				m_Ptr->AddRef();
			}
			if (old) {
				old->Release();
			}

			return *this;
		}

		template<typename CopyT>
		TRef& operator=(const TRef<CopyT>& other) {
			T* old = m_Ptr;
			m_Ptr = (T*)other.Get();

			if (m_Ptr) {
				m_Ptr->AddRef();
			}
			if (old) {
				old->Release();
			}

			return *this;
		}


		TRef& operator=(TRef&& move) {
			if (this != &move) {

				T* old = m_Ptr;
				m_Ptr = move.m_Ptr;
				move.m_Ptr = nullptr;

				if (old) {
					old->Release();
				}
			}

			return *this;
		}

		T* Get() const { return m_Ptr; }

		T* operator->() const {
			return m_Ptr;
		}

		operator T* () const {
			return m_Ptr;
		}

		bool operator==(const TRef<T>& other) const {
			return m_Ptr == other.m_Ptr;
		}

		bool operator==(T* other) const {
			return m_Ptr == other;
		}

		bool Valid() { return m_Ptr != nullptr; }

	private:
		mutable T* m_Ptr;

		template<typename OtherT>
		friend class TRef;
	};

	template<typename T, typename... TArgs>
	TRef<T> CreateRef(TArgs&&... args) {
		return TRef<T>(new T(std::forward<TArgs>(args)...));
	}

	// thread safe queue
	template<typename T>
	class TSafeQueue {
	public:
		TSafeQueue() {}
		~TSafeQueue() { m_Queue.clear(); }

		void Push(T&& element) {
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue.push_back(std::move(element));
		}

		void Push(const T& element) {
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue.push_back(element);
		}

		void PopAll(std::vector<T>& other) {
			std::lock_guard<std::mutex> lock(m_Mutex);
			other = std::move(m_Queue);
			m_Queue.clear();
		}

	private:
		std::vector<T> m_Queue;
		std::mutex m_Mutex;
	};
}