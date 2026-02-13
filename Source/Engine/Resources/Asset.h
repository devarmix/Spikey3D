#pragma once

#include <Engine/Core/Common.h>
#include <Engine/Core/RefCounted.h>

#include <EnkiTS/TaskScheduler.h>

namespace Spikey
{
	class Guid
	{
	private:
		uint64 m_Id = 0;

	public:
		Guid() = default;

		explicit Guid(uint64 id)
			: m_Id(id)
		{
		}

		bool operator==(const Guid& other) const
		{
			return m_Id == other.m_Id;
		}

		operator uint64() const
		{
			return m_Id;
		}

		bool IsValid() const
		{
			return m_Id != 0;
		}

		operator bool() const
		{
			return IsValid();
		}

	public:
		std::string ToString() const
		{
			return std::to_string(m_Id);
		}

		static Guid New();
	};
}

namespace std
{
	template<>
	struct hash<Spikey::Guid>
	{
		constexpr uint64 operator()(const Spikey::Guid& guid) const
		{
			return std::hash<uint64>{}((uint64)guid);
		}
	};
}

namespace Spikey
{
	class AssetTask
	{
	public:
		enum State
		{
			Created,
			Queued,
			Running,
			Canceled,
			Finished,
			Failed
		};

	protected:
		std::atomic<State>    m_State{ State::Created };
		std::atomic<uint32>   m_CancelFlag{ 0 };

		TSharedPtr<AssetTask> m_Next = nullptr;

		void SetState(State state)
		{
			m_State.store(state, std::memory_order_release);
		}

	public:
		State GetState() const
		{
			return m_State.load(std::memory_order_acquire);
		}

		TSharedPtr<AssetTask> GetNextTask() const
		{
			return m_Next;
		}

	public:
		bool IsQueued() const
		{
			return GetState() == State::Queued;
		}

		bool IsRunning() const
		{
			return GetState() == State::Running;
		}

		bool IsCanceled() const
		{
			return GetState() == State::Canceled;
		}

		bool IsFinished() const
		{
			return GetState() == State::Finished;
		}

		bool HasFailed() const
		{
			return GetState() == State::Failed;
		}

		bool IsCancelRequested() const
		{
			return m_CancelFlag.load(std::memory_order_acquire);
		}

		bool HasEnded() const
		{
			auto state = GetState();
			return
				state == State::Failed ||
				state == State::Canceled ||
				state == State::Finished;
		}

	public:
		void Start();
		void Cancel();
		bool Wait(double milliseconds) const;
		
		void SetNext(const TSharedPtr<AssetTask>& next);
		void Execute();

	protected:
		virtual void Enqueue() = 0;
		virtual bool Run() = 0;

		virtual void OnStart();
		virtual void OnFinish();
		virtual void OnFail();
		virtual void OnCancel();
		virtual void OnEnd();
	};

	class Asset
	{
		friend class AssetManager;

	public:
		enum Type
		{
			Unknown,
			Texture,
			Mesh,
			Material
		};

	protected:
		mutable std::atomic<uint32> m_RefCounter;
		std::atomic<uint32> m_LoadedFlag;

		std::mutex m_Locker;
		std::string m_Path;

		// AssetContentLoadTask* m_StreamingTask;
		Guid m_ID;

		Asset(const std::string& path, const Guid& id)
			: m_Path(path)
			, m_ID(id)
			, m_RefCounter(0)
			, m_LoadedFlag(0)
		{
		}

	public:
		virtual ~Asset() = default;

		virtual Type GetType() const
		{
			return Type::Unknown;
		}

		Guid GetGuid() const
		{
			return m_ID;
		}

		uint32 AddRef() const
		{
			return ++m_RefCounter;
		}

		uint32 Release() const
		{
			return --m_RefCounter;
		}

		uint32 GetRefCount() const
		{
			return m_RefCounter.load();
		}

		bool WaitForLoaded(double milliseconds);
		void Reload();
		void CancelStreaming();

		const std::string& GetPath() const
		{
			return m_Path;
		}

		bool IsLoaded() const
		{
			return m_LoadedFlag.load(std::memory_order_acquire) != 0;
		}

	protected:
		virtual void OnDestroy();
		virtual void Load() = 0;
		virtual void Unload(bool isReloading) = 0;
	};

	struct AssetHeader
	{
		static constexpr uint32 MAGIC_ID = 865467616;

		uint32 Magic = MAGIC_ID;
		Asset::Type Type;
		Guid ID;
	};
	static_assert(sizeof(AssetHeader) == 16 && "Invalid AssetHeader size!");

	class AssetManager
	{
	public:

		static TRefCountPtr<Asset> LoadAsset(Guid id);
		static TRefCountPtr<Asset> GetAsset(const std::string_view& path);

		static void UnloadAsset(Asset* asset);
		static void UnloadAllUnusedAssets();

		static void Tick();
		static void Init();
	};
}