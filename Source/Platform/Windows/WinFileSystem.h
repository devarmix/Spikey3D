#pragma once

#if SPIKEY_PLATFORM_WIN32
#include <Engine/Core/FileSystemBase.h>

namespace Spikey
{
	class WinFile : public FileBase
	{
	private:
		void* m_Handle;

	public:
		WinFile(void* handle)
			: m_Handle(handle)
		{
		}

		~WinFile();

	public:
		static WinFile* Open(const std::string_view& path, FileMode mode, FileAccess access = FileAccess::ReadWrite, FileShare = FileShare::None);

	public:
		virtual bool Read(void* buffer, uint32 bytesToRead, uint32* bytesRead = nullptr) override;
		virtual bool Write(const void* buffer, uint32 bytesToWrite, uint32* bytesWritten = nullptr) override;
		virtual void Close() final override;
		virtual uint32 GetSize() const override;
		virtual DateTime GetLastWriteTime() const override;
		virtual uint32 GetPosition() const override;
		virtual void SetPosition(uint32 seek) override;
		virtual bool IsOpened() const override;
	};

	class WinFileSystem : public FileSystemBase
	{
	public:
		static bool CreateDirectory(const std::string_view& path);
		static bool DeleteDirectory(const std::string_view& path);
		static bool DirectoryExists(const std::string_view& path);
		static bool IterateDirectory(const std::string_view& path, DirectoryVisitor& visitor);
		static bool FileExists(const std::string_view& path);
		static bool DeleteFile(const std::string_view& path);
		static uint64 GetFileSize(const std::string_view& path);
		static bool IsReadOnly(const std::string_view& path);
		static bool SetReadOnly(const std::string_view& path, bool isReadOnly);
		static bool MoveFile(const std::string_view& dst, const std::string_view& src, bool overwrite = false);
		static bool CopyFile(const std::string_view& dst, const std::string_view& src);
		static DateTime GetFileLastEditTime(const std::string_view& path);

	private:
		static bool IterateDirectoryCommon(const std::string_view& path, const std::function<bool(const WIN32_FIND_DATAW&)>& visitor);
	};
}

#endif