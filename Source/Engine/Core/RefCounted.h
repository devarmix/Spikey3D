#pragma once

#include <Engine/Core/Common.h>

namespace Spikey 
{
	class RefCounted 
	{
	public:
		virtual ~RefCounted() = default;

		virtual uint32 AddRef() const 
		{
			return ++m_Counter;
		}

		virtual uint32 Release() const 
		{
			uint32 count = --m_Counter;
			if (count == 0)
			{
				delete this;
			}
			return count;
		}

		uint32 GetRefCount() const { return m_Counter.load(); }

	protected:
		mutable std::atomic<uint32> m_Counter{ 0 };
	};

	// UE4 like reference counting
	template<typename T>
	class TRefCountPtr
	{
	public:
		TRefCountPtr() 
			: m_Reference(nullptr)
		{
		}

		TRefCountPtr(T* ptr, bool addRef = true) 
		{
			m_Reference = ptr;
			if (m_Reference && addRef)
			{
				m_Reference->AddRef();
			}
		}

		TRefCountPtr(const TRefCountPtr& copy) 
		{
			m_Reference = copy.m_Reference;
			if (m_Reference)
			{
				m_Reference->AddRef();
			}
		}

		template<typename CopyT>
		explicit TRefCountPtr(const TRefCountPtr<CopyT>& copy) 
		{
			m_Reference = static_cast<T*>(copy.Get());
			if (m_Reference)
			{
				m_Reference->AddRef();
			}
		}

		TRefCountPtr(TRefCountPtr&& move) noexcept 
		{
			m_Reference = move.m_Reference;
			move.m_Reference = nullptr;
		}

		template<typename MoveT>
		explicit TRefCountPtr(TRefCountPtr<MoveT>&& move) 
		{
			m_Reference = static_cast<T*>(move.Get());
			move.m_Reference = nullptr;
		}

		~TRefCountPtr() 
		{
			if (m_Reference)
			{
				m_Reference->Release();
			}
		}

		TRefCountPtr& operator=(T* ptr) 
		{
			T* oldReference = m_Reference;
			m_Reference = ptr;

			if (m_Reference)
			{
				m_Reference->AddRef();
			}
			if (oldReference)
			{
				oldReference->Release();
			}
			return *this;
		}

		TRefCountPtr& operator=(const TRefCountPtr& other) 
		{
			return *this = other.m_Reference;
		}

		template<typename CopyT>
		TRefCountPtr& operator=(const TRefCountPtr<CopyT>& other) 
		{
			return *this = other.Get();
		}

		TRefCountPtr& operator=(TRefCountPtr&& move) 
		{
			if (this != &move)
			{
				T* oldReference = m_Reference;
				m_Reference = move.m_Reference;
				move.m_Reference = nullptr;

				if (oldReference)
				{
					oldReference->Release();
				}
			}
			return *this;
		}

		T* operator->() const 
		{
			return m_Reference;
		}

		operator T*() const 
		{
			return m_Reference;
		}

		bool operator==(const TRefCountPtr<T>& other) const 
		{
			return m_Reference == other.m_Reference;
		}

		bool operator==(T* other) const 
		{
			return m_Reference == other;
		}

		T* Get() const 
		{ 
			return m_Reference;
		}

		bool Valid() const
		{
			return m_Reference != nullptr;
		}

		uint32 GetRefCount()
		{
			uint32 result = 0;
			if (m_Reference)
			{
				result = m_Reference->GetRefCount();
				assert(result > 0);
			}
			return result;
		}

		void Swap(TRefCountPtr& other)
		{
			T* oldReference = m_Reference;
			m_Reference = other.m_Reference;
			other.m_Reference = oldReference;
		}

	private:
		mutable T* m_Reference;

		template<typename OtherT>
		friend class TRefCountPtr;
	};
}