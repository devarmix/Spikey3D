#pragma once
#include <Engine/Core/Common.h>

namespace Spikey {

	class IRefCounted {
	public:
		virtual ~IRefCounted() = default;

		virtual void AddRef() const {
			++m_Counter;
		}

		virtual void Release() const {
			if (--m_Counter == 0) {
				delete this;
			}
		}

		uint32 GetRefCount() const { return m_Counter.load(); }
	protected:
		mutable std::atomic<uint32> m_Counter{ 0 };
	};

	// for IRefCounted derived classes
	template<typename T>
	class TRefCountPtr {
	public:
		TRefCountPtr() : m_Ptr(nullptr) {}
		TRefCountPtr(T* ptr) {
			m_Ptr = ptr;
			if (m_Ptr) {
				m_Ptr->AddRef();
			}
		}

		TRefCountPtr(const TRefCountPtr& copy) {
			m_Ptr = copy.m_Ptr;
			if (m_Ptr) {
				m_Ptr->AddRef();
			}
		}

		template<typename CopyT>
		explicit TRefCountPtr(const TRefCountPtr<CopyT>& copy) {
			m_Ptr = static_cast<T*>(copy.Get());
			if (m_Ptr) {
				m_Ptr->AddRef();
			}
		}

		template<typename CopyT>
		TRefCountPtr<CopyT> As() const {
			return TRefCountPtr<CopyT>(*this);
		}

		TRefCountPtr(TRefCountPtr&& move) noexcept {
			m_Ptr = move.m_Ptr;
			move.m_Ptr = nullptr;
		}

		template<typename MoveT>
		explicit TRefCountPtr(TRefCountPtr<MoveT>&& move) {
			m_Ptr = static_cast<T*>(move.Get());
			move.m_Ptr = nullptr;
		}

		~TRefCountPtr() {
			if (m_Ptr) {
				m_Ptr->Release();
			}
		}

		TRefCountPtr& operator=(T* ptr) {
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

		TRefCountPtr& operator=(const TRefCountPtr& other) {
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
		TRefCountPtr& operator=(const TRefCountPtr<CopyT>& other) {
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


		TRefCountPtr& operator=(TRefCountPtr&& move) {
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
		friend class TRefCountPtr;
	};

	template<typename T, typename... TArgs>
	TRefCountPtr<T> CreateRefCounted(TArgs&&... args) {
		return TRefCountPtr<T>(new T(std::forward<TArgs>(args)...));
	}
}