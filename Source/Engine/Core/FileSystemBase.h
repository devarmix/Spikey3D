#pragma once

#include <Engine/Core/Common.h>
#include <Engine/Core/Time.h>

namespace Spikey
{
	enum class FileMode : uint32
	{
		CreateNew = 1,
		CreateAlways = 2,
		OpenExisting = 3,
		OpenAlways = 4,
		TruncateExisting = 5
	};

	enum class FileAccess : uint32
	{
		Read = 0x80000000,
		Write = 0x40000000,
		ReadWrite = Read | Write
	};

	enum class FileShare : uint32
	{
		None = 0x00000000,
		Read = 0x00000001,
		Write = 0x00000002,
		Delete = 0x00000004,

		ReadWrite = Read | Write,
		All = ReadWrite | Delete
	};

	class FileBase
	{
	protected:
		FileBase() = default;

	public:
		virtual ~FileBase()
		{
		}

		FileBase(const FileBase& other) = delete;
		FileBase& operator=(const FileBase& other) = delete;

	public:
		virtual bool Read(void* buffer, uint32 bytesToRead, uint32* bytesRead = nullptr) = 0;
		virtual bool Write(const void* buffer, uint32 bytesToWrite, uint32* bytesWritten = nullptr) = 0;
		virtual void Close() = 0;

	public:
		virtual uint32 GetSize() const = 0;
		virtual DateTime GetLastWriteTime() const = 0;
		virtual uint32 GetPosition() const = 0;
		virtual void SetPosition(uint32 seek) = 0;
		virtual bool IsOpened() const = 0;
	};

	class DirectoryVisitor
	{
	public:
		DirectoryVisitor()
		{
		}

		virtual ~DirectoryVisitor()
		{
		}

		virtual bool Visit(const std::string_view& path, bool isDirectory) = 0;
	};

	class FileSystemBase
	{
	public:
		static bool CreateDirectory(const std::string_view& path) = delete;
		static bool DeleteDirectory(const std::string_view& path) = delete;
		static bool DirectoryExists(const std::string_view& path) = delete;
		static bool IterateDirectory(const std::string_view& path, DirectoryVisitor& visitor) = delete;

	public:
		static bool FileExists(const std::string_view& path) = delete;
		static bool DeleteFile(const std::string_view& path) = delete;
		static uint64 GetFileSize(const std::string_view& path) = delete;
		static bool IsReadOnly(const std::string_view& path) = delete;
		static bool SetReadOnly(const std::string_view& path, bool isReadOnly) = delete;
		static bool MoveFile(const std::string_view& dst, const std::string_view& src, bool overwrite = false) = delete;
		static bool CopyFile(const std::string_view& dst, const std::string_view& src) = delete;
		static DateTime GetFileLastEditTime(const std::string_view& path) = delete;

	public:
		static bool IsRelative(const std::string_view& path);
		static void GetTempFolderPath(std::string& result);

		static std::string GetExtension(const std::string_view& path);
		static std::string ConvertRelativeToAbsolute(const std::string_view& path);
		static std::string ConvertRelativeToAbsolute(const std::string_view& path, const std::string_view& basePath);
	};

	// path operators for strings
	inline std::string& operator/=(std::string& lhs, const char* rhs);
	inline std::string& operator/=(std::string& lhs, const std::string_view& rhs);
	inline std::string& operator/=(std::string& lhs, const std::string& rhs);
	inline std::string  operator/(std::string&& lhs, const char* rhs);
	inline std::string  operator/(std::string&& lhs, const std::string_view& rhs);
	inline std::string  operator/(std::string&& lhs, const std::string& rhs);
	inline std::string  operator/(const std::string& lhs, const char* rhs);
	inline std::string  operator/(const std::string& lhs, const std::string_view& rhs);
	inline std::string  operator/(const std::string& lhs, const std::string& rhs);
}