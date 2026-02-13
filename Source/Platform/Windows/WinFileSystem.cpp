#if SPIKEY_PLATFORM_WIN32

#include <Platform/Windows/WinFileSystem.h>
#include <Platform/Windows/WindowsHeaders.h>
#include <Engine/Core/StringConversions.h>
#include <tchar.h>

namespace Spikey
{
	class WindowsNormalizedPath
	{
	private:
		StringAsWChar<> m_Path;

	public:
		WindowsNormalizedPath(const std::string_view& path)
		{
			std::string fullPath = WinFileSystem::ConvertRelativeToAbsolute(path);

			for (char& c : fullPath)
			{
				if (c == '/')
				{
					c = '\\';
				}
			}

			if (fullPath.size() > MAX_PATH)
			{
				fullPath = "\\\\?\\" + fullPath;
			}

			m_Path = fullPath.c_str();
		}

		operator const wchar_t*() const
		{
			return m_Path.Get();
		}
	};

	WinFile::~WinFile()
	{
		Close();
	}

	WinFile* WinFile::Open(const std::string_view& path, FileMode mode, FileAccess access, FileShare share)
	{
		HANDLE handle = CreateFileW(
			WindowsNormalizedPath(path),
			(DWORD)access,
			(DWORD)share,
			nullptr,
			(DWORD)mode,
			FILE_ATTRIBUTE_NORMAL,
			nullptr);

		if (handle == INVALID_HANDLE_VALUE)
		{
			return nullptr;
		}

		return new WinFile(handle);
	}

	bool WinFile::Read(void* buffer, uint32 bytesToRead, uint32* bytesRead)
	{
		DWORD tmp;
		if (ReadFile(m_Handle, buffer, bytesToRead, &tmp, nullptr))
		{
			if (bytesRead)
				*bytesRead = tmp;
			return true;
		}

		if (bytesRead)
			*bytesRead = 0;
		return false;
	}

	bool WinFile::Write(const void* buffer, uint32 bytesToWrite, uint32* bytesWritten)
	{
		DWORD tmp;
		if (WriteFile(m_Handle, buffer, bytesToWrite, &tmp, nullptr))
		{
			if (bytesWritten)
				*bytesWritten = tmp;
			return true;
		}

		if (bytesWritten)
			*bytesWritten = 0;
		return false;
	}

	void WinFile::Close()
	{
		if (m_Handle)
		{
			CloseHandle(m_Handle);
			m_Handle = nullptr;
		}
	}

	uint32 WinFile::GetSize() const
	{
		LARGE_INTEGER result;
		GetFileSizeEx(m_Handle, &result);
		return (uint32)result.QuadPart;
	}

	DateTime WinFile::GetLastWriteTime() const
	{
		FILETIME lastWriteTime;
		GetFileTime(m_Handle, nullptr, nullptr, &lastWriteTime);

		SYSTEMTIME systemTime;
		FileTimeToSystemTime(&lastWriteTime, &systemTime);

		return DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
	}

	uint32 WinFile::GetPosition() const
	{
		return SetFilePointer(m_Handle, 0, nullptr, FILE_CURRENT);
	}

	void WinFile::SetPosition(uint32 seek)
	{
		SetFilePointer(m_Handle, seek, nullptr, FILE_BEGIN);
	}

	bool WinFile::IsOpened() const
	{
		return m_Handle != nullptr;
	}

	bool WinFileSystem::CreateDirectory(const std::string_view& path)
	{
		const DWORD fileAttributes = GetFileAttributesW(WindowsNormalizedPath(path));
		if (fileAttributes == INVALID_FILE_ATTRIBUTES)
		{
			const auto error = GetLastError();
			if (error == ERROR_ACCESS_DENIED)
				return false;

			int32 slashIndex = -1;
			for (int32 i = path.size() - 1; i >= 0; i--)
			{
				if (path[i] == '/' || path[i] == '\\')
				{
					slashIndex = i;
					break;
				}
			}

			// create parent directories recursively
			if (slashIndex != -1)
			{
				if (!CreateDirectory(path.substr(0, slashIndex)))
				{
					return false;
				}
			}

			const BOOL result = CreateDirectoryW(WindowsNormalizedPath(path), nullptr);
			if (result == FALSE)
			{
				return false;
			}
		}
		else
		{
			if (!((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 || (fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0))
			{
				return false;
			}
		}

		return true;
	}

	bool WinFileSystem::DeleteDirectory(const std::string_view& path)
	{
		RemoveDirectoryW(WindowsNormalizedPath(path));
		const bool succeeded = !DirectoryExists(path);

		return succeeded;
	}

	bool WinFileSystem::DirectoryExists(const std::string_view& path)
	{
		const DWORD result = GetFileAttributesW(WindowsNormalizedPath(path));
		return result != 0xFFFFFFFF && result & FILE_ATTRIBUTE_DIRECTORY;
	}

	bool WinFileSystem::FileExists(const std::string_view& path)
	{
		const DWORD result = GetFileAttributesW(WindowsNormalizedPath(path));
		return result != 0xFFFFFFFF && !(result & FILE_ATTRIBUTE_DIRECTORY);
	}

	bool WinFileSystem::DeleteFile(const std::string_view& path)
	{
		return !!DeleteFileW(WindowsNormalizedPath(path));
	}

	uint64 GetFileSize(const std::string_view& path)
	{
		WIN32_FILE_ATTRIBUTE_DATA info;
		if (!!GetFileAttributesExW(WindowsNormalizedPath(path), GetFileExInfoStandard, &info))
		{
			if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
				LARGE_INTEGER li;
				li.HighPart = info.nFileSizeHigh;
				li.LowPart = info.nFileSizeLow;
				return li.QuadPart;
			}
		}

		return 0;
	}

	bool WinFileSystem::IsReadOnly(const std::string_view& path)
	{
		const DWORD result = GetFileAttributesW(WindowsNormalizedPath(path));
		return result != 0xFFFFFFFF ? !!(result & FILE_ATTRIBUTE_READONLY) : false;
	}

	bool WinFileSystem::SetReadOnly(const std::string_view& path, bool isReadOnly)
	{
		return SetFileAttributesW(WindowsNormalizedPath(path), isReadOnly ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL) == 0;
	}

	bool WinFileSystem::MoveFile(const std::string_view& dst, const std::string_view& src, bool overwrite)
	{
		const DWORD flags = MOVEFILE_COPY_ALLOWED | (overwrite ? MOVEFILE_REPLACE_EXISTING : 0);
		return !!MoveFileExW(WindowsNormalizedPath(src), WindowsNormalizedPath(dst), flags);
	}

	bool WinFileSystem::CopyFile(const std::string_view& dst, const std::string_view& src)
	{
		return !!CopyFileW(WindowsNormalizedPath(src), WindowsNormalizedPath(dst), FALSE);
	}

	DateTime WinFileSystem::GetFileLastEditTime(const std::string_view& path)
	{
		DateTime result = DateTime::MinValue();

		WIN32_FILE_ATTRIBUTE_DATA data;
		if (!!GetFileAttributesExW(WindowsNormalizedPath(path), GetFileExInfoStandard, &data))
		{
			SYSTEMTIME systemTime;
			FileTimeToSystemTime(&data.ftLastWriteTime, &systemTime);

			result = DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond, systemTime.wMilliseconds);
		}

		return result;
	}

	bool WinFileSystem::IterateDirectoryCommon(const std::string_view& path, const std::function<bool(const WIN32_FIND_DATAW&)>& visitor)
	{
		bool result = true;
		WIN32_FIND_DATAW data;
		std::string searchWildcard = std::string(path) / "*.*";
		HANDLE handle = FindFirstFileW(WindowsNormalizedPath(searchWildcard), &data);

		if (handle != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L".."))
				{
					result = visitor(data);
				}
			} while (result && FindNextFileW(handle, &data));
			FindClose(handle);
		}

		return result;
	}

	bool WinFileSystem::IterateDirectory(const std::string_view& path, DirectoryVisitor& visitor)
	{
		const std::string directoryStr(path);
		return IterateDirectoryCommon(path, [&](const WIN32_FIND_DATAW& data) -> bool
			{
				const bool isDirectory = !!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
				std::string iteratorPath;
				{
					StringAsUtf8<> filename(data.cFileName);
					iteratorPath = directoryStr / std::string_view(filename.Get(), filename.Length());
				}
				return visitor.Visit(iteratorPath, isDirectory);
			});
	}
}

#endif