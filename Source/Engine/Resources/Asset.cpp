#include <Engine/Resources/Asset.h>
#include <Engine/Core/Time.h>
#include <Engine/Resources/FileStream.h>
#include <Engine/Core/Engine.h>

#if WITH_EDITOR
#include <FileWatcher/FileWatch.hpp>
#endif

namespace Spikey
{
	std::mutex s_AssetLocker;
	std::unordered_map<Guid, Asset*> s_Assets(2048);
	std::vector<Asset*> s_ToUnload;
	std::unordered_map<Asset*, TimeSpan> s_UnloadQueue;
	std::mutex s_LoadedAssetsToInvokeLocker;
	std::vector<Asset*> s_LoadedAssetsToInvoke;

	TimeSpan s_AssetUpdateInterval = TimeSpan::FromMilliseconds(500);
	TimeSpan s_AssetUnloadInterval = TimeSpan::FromSeconds(10);
	TimeSpan s_LastUnloadCheckTime = TimeSpan::Zero();

#if WITH_EDITOR
#define ASSET_FILE_EXTENSION ".asset"

	static uint64 GetLastEditTime(const std::filesystem::path& file)
	{

	}

	static bool GetAssetHeader(const std::filesystem::path& file, AssetHeader& header)
	{
		if (!std::filesystem::exists(file))
			return false;

		FileReadStream stream(file);
		if (!stream.IsOpen())
		{
			ENGINE_ERROR("Failed to open asset file at: {}", file.string());
			return false;
		}

		stream.Read(header.Magic);
		if (header.Magic != AssetHeader::MAGIC_ID)
		{
			ENGINE_ERROR("Invalid asset file at: {}", file.string());
			return false;
		}

		stream.Read(header.Type);

		uint64 id;
		stream.Read(id);
		header.ID = Guid(id);

		return !stream.HasError();
	}

	struct AssetUpdateRequest
	{
		std::function<void()> Task;
		std::atomic<uint64> FinishedFlag{ 0 };

		void Wait() const
		{
			while (FinishedFlag.load(std::memory_order_acquire) == 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	};

	std::mutex s_AssetUpdateQueueLocker;
	std::vector<AssetUpdateRequest> s_AssetUpdateQueue;

	constexpr char ASSET_CACHE_FILE_ID[4] = { 'S', 'A', 'C', 'F' };

	class AssetCache
	{
	public:
		struct AssetInfo
		{
			Guid ID;
			Asset::Type Type;
			uint64 FileModified;
			std::string SourcePath;
			std::string Path;
		};

	private:
		bool m_IsDirty = false;
		std::mutex m_Lock;
		std::filesystem::path m_Path;
		std::unordered_map<Guid, AssetInfo> m_Registry;

	public:
		void Init()
		{
			m_Path = Engine::ProjectCacheFolder / "AssetsCache.dat";
			ENGINE_TRACE("Loading asset cache from: {}", m_Path.string());

			if (!std::filesystem::exists(m_Path))
			{
				m_IsDirty = true;
				ENGINE_WARN("Cannot find assets cache file");
				return;
			}

			FileReadStream stream(m_Path);

			uint32 id;
			stream.Read(id);
			if (memcmp(&id, ASSET_CACHE_FILE_ID, sizeof(uint32)) != 0)
			{
				ENGINE_WARN("Corrupted asset cache file");
				return;
			}

			uint32 count;
			stream.Read(count);
			m_Registry.clear();
			m_Registry.reserve(count);

			uint32 invalidCount = 0;
			AssetInfo e = {};

			for (uint32 i = 0; i < count; i++)
			{
				uint64 id;
				stream.Read(id);
				stream.Read(e.Type);
				stream.Read(e.SourcePath);
				stream.Read(e.Path);

				e.ID = Guid(id);

				if (IsEntryValid(e))
					m_Registry.emplace(e.ID, e);
				else
					invalidCount++;
			}

			if (stream.HasError())
			{
				m_IsDirty = true;
				m_Registry.clear();
				ENGINE_WARN("Asset cache file has an error. Removing it");

				if (!std::filesystem::remove(m_Path))
				{
					ENGINE_WARN("Failed to remove asset cache file!");
				}
			}

			ENGINE_TRACE("Asset cache loaded with {0} entries, ({1} invalid)", m_Registry.size(), invalidCount);
		}

		bool Save()
		{
			std::scoped_lock lock(m_Lock);

			if (!m_IsDirty && std::filesystem::exists(m_Path))
				return false;

			ENGINE_TRACE("Saving asset cache to: {0}, num entries: {1}", m_Path.string(), m_Registry.size());

			FileWriteStream stream(m_Path);
			if (!stream.IsOpen())
				return false;

			stream.WriteBytes(ASSET_CACHE_FILE_ID, sizeof(ASSET_CACHE_FILE_ID));
			stream.Write((uint32)m_Registry.size());

			for (auto i = m_Registry.begin(); i != m_Registry.end(); i++)
			{
				auto& e = i->second;
				stream.Write((uint64)e.ID);
				stream.Write(e.Type);
				stream.Write(e.SourcePath);
				stream.Write(e.Path);
			}

			m_IsDirty = false;
			return true;
		}

		bool FindAsset(const Guid& id, AssetInfo& info)
		{
			bool result = false;
			std::scoped_lock lock(m_Lock);

			auto it = m_Registry.find(id);
			if (it != m_Registry.end())
			{
				AssetInfo& e = it->second;

				if (!IsEntryValid(e))
				{
					ENGINE_WARN("Missing file from registry: {}", e.Path);
					m_Registry.erase(it);
				}
				else
				{
					result = true;
					info = e;
				}
			}

			return result;
		}

		bool FindAsset(const std::string_view& path, AssetInfo& info)
		{
			bool result = false;
			std::scoped_lock lock(m_Lock);

			for (auto i = m_Registry.begin(); i != m_Registry.end(); i++)
			{
				auto& e = i->second;
				if (e.Path == path)
				{
					if (!IsEntryValid(e))
					{
						ENGINE_WARN("Missing file from registry: {}", e.Path);
						m_Registry.erase(i);
					}
					else
					{
						result = true;
						info = e;
					}

					break;
				}
			}

			return result;
		}

		void RegisterAsset(const Guid& id, Asset::Type type, const std::string_view& sourcePath, const std::string_view& path)
		{
			std::scoped_lock lock(m_Lock);

			bool isMissing = true;
			for (auto i = m_Registry.begin(); i != m_Registry.end(); i++)
			{
				auto& e = i->second;

				if (e.ID == id)
				{
					if (e.Path != path)
					{
						e.Path = path;
						m_IsDirty = true;
					}
					if (e.Type != type)
					{
						e.Type = type;
						m_IsDirty = true;
					}
					if (e.SourcePath != sourcePath)
					{
						e.SourcePath = sourcePath;
						m_IsDirty = true;
					}
					isMissing = false;
					break;
				}

				if (e.Path == path)
				{
					if (e.ID != id)
					{
						e.ID = id;
						m_IsDirty = true;
					}
					if (e.Type != type)
					{
						e.Type = type;
						m_IsDirty = true;
					}
					if (e.SourcePath != sourcePath)
					{
						e.SourcePath = sourcePath;
						m_IsDirty = true;
					}
					isMissing = false;
					break;
				}
			}

			if (isMissing)
			{
				ENGINE_TRACE("Registring an asset: {0}, {1}", (uint64)id, path);

				AssetInfo info{};
				info.ID = id;
				info.Type = type;
				info.Path = path;
				info.SourcePath = sourcePath;

				m_Registry.emplace(id, std::move(info));
				m_IsDirty = true;
			}
		}

		bool DeleteAsset(const std::string_view& path)
		{
			bool result = false;
			std::scoped_lock lock(m_Lock);

			for (auto i = m_Registry.begin(); i != m_Registry.end(); i++)
			{
				auto& e = i->second;
				if (e.Path == path)
				{
					m_Registry.erase(i);
					m_IsDirty = true;
					result = true;
					break;
				}
			}

			return result;
		}

		bool DeleteAsset(const Guid& id)
		{
			bool result = false;
			std::scoped_lock lock(m_Lock);

			auto it = m_Registry.find(id);
			if (it != m_Registry.end())
			{
				m_Registry.erase(it);
				m_IsDirty = true;
				result = true;
			}

			return result;
		}

		bool RenameAsset(const std::string_view& oldPath, const std::string_view& newPath)
		{
			bool result = false;
			std::scoped_lock lock(m_Lock);

			for (auto i = m_Registry.begin(); i != m_Registry.end(); i++)
			{
				auto& e = i->second;
				if (e.Path == oldPath)
				{
					e.Path = newPath;
					m_IsDirty = true;
					result = true;
					break;
				}
			}

			return result;
		}

		uint32 Size() const
		{
			std::scoped_lock locK(m_Lock);
			return m_Registry.size();
		}

		bool IsEntryValid(AssetInfo& entry)
		{
			const std::filesystem::path path = entry.Path;

			if (std::filesystem::exists(path))
			{
				const uint64 fileModified = GetLastEditTime(path);
				if (fileModified == entry.FileModified)
					return true;

				const std::string extension = path.extension().string();

				if (extension == ASSET_FILE_EXTENSION)
				{
					AssetHeader header;
					bool isValid = GetAssetHeader(path, header);

					if (header.Type != entry.Type || header.ID != entry.ID)
					{
						isValid = false;
					}

					entry.FileModified = fileModified;
					m_IsDirty = true;

					return isValid;
				}
			}

			return false;
		}
	};

	AssetCache s_Cache;

	class AssetImportManager
	{
	public:
		struct AssetImporter
		{
			std::string FileExtension;
			std::string ResultExtension;

			std::function<bool(const std::filesystem::path&, const std::filesystem::path&, const Guid&)> CreateCallback;
		};

	private:
		std::vector<AssetImporter> m_Importers;

	public:
		void Init()
		{
			m_Importers =
			{
				{".png", ASSET_FILE_EXTENSION, [](const std::filesystem::path& path, const std::filesystem::path& path2, const Guid& id)
				{
					return false;
				}}
			};
		}

		const AssetImporter* GetImporter(const std::string_view& extension)
		{
			for (auto& importer : m_Importers)
			{
				if (importer.FileExtension == extension)
				{
					return &importer;
				}
			}

			return nullptr;
		}

		bool Import(const std::filesystem::path& inputPath, const std::filesystem::path& outputPath, Guid& assetID, void* args)
		{
			ENGINE_TRACE("Importing file {0} to {1} ...", inputPath.string(), outputPath.string());

			if (!std::filesystem::exists(inputPath))
			{
				ENGINE_ERROR("Missing file {}", inputPath.string());
				return false;
			}

			const std::string extension = inputPath.extension().string();
			const auto importer = GetImporter(extension);

			if (importer == nullptr)
			{
				ENGINE_ERROR("Cannot import file {}. Unknown file type", inputPath.string());
				return false;
			}

			assetID = Guid::New();

			// check if asset already exists in project
			AssetCache::AssetInfo info = {};
			if (s_Cache.FindAsset(outputPath.string(), info))
			{
				assetID = info.ID;
			}
			else
			{
				const std::filesystem::path outputDirectory = outputPath.root_directory();
				if (!std::filesystem::create_directories(outputDirectory))
				{
					ENGINE_WARN("Failed to create directory: {0}", outputDirectory.string());
				}
			}

			std::filesystem::path tempAssetPath
				= std::filesystem::temp_directory_path() / "Spikey3D" / (Guid::New().ToString() + ASSET_FILE_EXTENSION);

			bool result = importer->CreateCallback(inputPath, tempAssetPath, assetID);
			if (!result)
			{
				std::filesystem::remove(tempAssetPath);
				return false;
			}
			{
				s_AssetUpdateQueueLocker.lock();

				AssetUpdateRequest& request
					= s_AssetUpdateQueue.emplace_back();
				s_AssetUpdateQueueLocker.unlock();

				request.Task = [&]() {
					auto asset = AssetManager::GetAsset(outputPath.string());
					if (asset)
					{
						asset->CancelStreaming();
					}

					if (!std::filesystem::copy_file(tempAssetPath, outputPath, std::filesystem::copy_options::overwrite_existing))
					{
						// copy failed so wait here
					}

					if (asset)
					{
						asset->Reload();
					}
					};

				request.Wait();
			}

			s_Cache.RegisterAsset(assetID, type, s, p);
			std::filesystem::remove(tempAssetPath);

			return true;
		}
	};

	AssetImportManager s_ImportManager;
#endif

	void AssetTask::Start()
	{
		assert(GetState() == State::Created);

		OnStart();
		SetState(State::Queued);
		Enqueue();
	}

	void AssetTask::Cancel()
	{
		if (m_CancelFlag.load(std::memory_order_acquire) == 0)
		{
			OnCancel();

			if (m_Next)
				m_Next->Cancel();
		}
	}

	bool AssetTask::Wait(double milliseconds) const
	{
		const auto startTime = std::chrono::high_resolution_clock::now();

		do
		{
			auto state = GetState();

			if (state == State::Finished)
			{
				if (m_Next)
				{
					const auto spendTime =
						std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();

					return m_Next->Wait(milliseconds - spendTime);
				}

				return true;
			}

			if (state == State::Canceled || state == State::Failed)
				return true;

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

		} while (milliseconds <= 0.0 ||
			std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count() < milliseconds);

		return false;
	}

	void AssetTask::SetNext(const TSharedPtr<AssetTask>& next)
	{
		assert(next != nullptr && next.get() != this);
		if (m_Next)
		{
			m_Next->SetNext(next);
		}
		else
		{
			m_Next = next;
		}
	}

	void AssetTask::Execute()
	{
		if (IsCanceled())
			return;

		assert(IsQueued());
		SetState(State::Running);

		bool failed = Run();

		if (IsCancelRequested())
		{
			SetState(State::Canceled);
		}
		else if (failed)
		{
			OnFail();
		}
		else
		{
			OnFinish();
		}
	}

	void AssetTask::OnStart()
	{
	}

	void AssetTask::OnEnd()
	{
	}

	void AssetTask::OnFinish()
	{
		assert(IsRunning() && !IsCancelRequested());
		SetState(State::Finished);

		if (m_Next)
			m_Next->Start();

		OnEnd();
	}

	void AssetTask::OnFail()
	{
		SetState(State::Failed);

		if (m_Next)
			m_Next->OnFail();

		OnEnd();
	}

	void AssetTask::OnCancel()
	{
		m_CancelFlag.store(1, std::memory_order_release);

		if (IsRunning())
		{
			const double timeout = 10000; // 10s
			
			if (!Wait(timeout))
			{
				assert(false
					&& "Task timeouted while waiting!");
			}
		}

		auto state = GetState();
		if (state != State::Finished && state != State::Failed)
		{
			SetState(State::Canceled);
			OnEnd();
		}
	}

	static bool IsImageFile(const std::string_view& extension)
	{
		std::array<const char*, 3> extensions =
		{
			".png",
			".hdr",
			".tga"
		};

		for (auto ext : extensions)
		{
			if (extension == ext)
			{
				return true;
			}
		}

		return false;
	}

	static bool IsMeshFile(const std::string_view& extension)
	{
		std::array<const char*, 4> extensions =
		{
			".fbx",
			".obj",
			".glb",
			".gltf"
		};

		for (auto ext : extensions)
		{
			if (extension == ext)
			{
				return true;
			}
		}

		return false;
	}

	void AssetManager::Init()
	{
#if WITH_EDITOR
		s_Cache.Init();
#endif
	}

	void AssetManager::Tick()
	{
		{
			std::scoped_lock lock(s_LoadedAssetsToInvokeLocker);

			while (!s_LoadedAssetsToInvoke.empty())
			{
				auto asset = s_LoadedAssetsToInvoke.back();
				s_LoadedAssetsToInvoke.pop_back();
				asset->OnLoaded();
			}
		}

		// unload unused resources
		{
			const TimeSpan timeNow = Time::UnscaledTime;
			if (timeNow - s_LastUnloadCheckTime < s_AssetUpdateInterval)
				return;

			s_LastUnloadCheckTime = timeNow;
			s_AssetLocker.lock();

			for (auto i = s_Assets.begin(); i != s_Assets.end(); i++)
			{
				Asset* asset = i->second;

				if (asset->GetRefCount() == 0 && !s_UnloadQueue.contains(asset))
				{
					s_UnloadQueue.emplace(asset, timeNow);
				}
			}

			for (auto i = s_UnloadQueue.begin(); i != s_UnloadQueue.end(); i++)
			{
				if (i->first->GetRefCount() > 0 || timeNow - i->second >= s_AssetUnloadInterval)
				{
					s_ToUnload.push_back(i->first);
				}
			}

			for (int32 i = 0; i < s_ToUnload.size(); i++)
			{
				Asset* asset = s_ToUnload[i];

				if (asset->GetRefCount() == 0)
				{
					UnloadAsset(asset);
				}

				s_UnloadQueue.erase(asset);
			}

			s_ToUnload.clear();
			s_AssetLocker.unlock();
		}
	}

	void AssetManager::UnloadAllUnusedAssets()
	{
		s_LastUnloadCheckTime = Time::UnscaledTime;
		s_AssetLocker.lock();

		for (auto i = s_Assets.begin(); i != s_Assets.end(); i++)
		{
			Asset* asset = i->second;

			if (asset->GetRefCount() == 0)
			{
				s_ToUnload.push_back(asset);
			}
		}

		for (int32 i = 0; i < s_ToUnload.size(); i++)
		{
			Asset* asset = s_ToUnload[i];
			UnloadAsset(asset);
		}

		s_UnloadQueue.clear();
		s_ToUnload.clear();
		s_AssetLocker.unlock();
	}

	TRefCountPtr<Asset> AssetManager::LoadAsset(Guid id)
	{

	}

	TRefCountPtr<Asset> AssetManager::GetAsset(const std::string_view& path)
	{
		if (path.empty())
			return nullptr;

		std::scoped_lock lock(s_AssetLocker);
		for (auto i = s_Assets.begin(); i != s_Assets.end(); i++)
		{
			if (i->second->GetPath() == path)
			{
				return i->second;
			}
		}

		return nullptr;
	}

	void AssetManager::UnloadAsset(Asset* asset)
	{
		if (asset == nullptr)
			return;

		asset->OnDestroy();
		delete asset;
	}
}