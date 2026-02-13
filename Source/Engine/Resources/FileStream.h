#pragma once

#include <fstream>
#include <Engine/Core/Common.h>

namespace Spikey 
{
	class FileReadStream
	{
	private:
		std::ifstream m_Stream;

	public:
		FileReadStream(const std::filesystem::path& path)
			: m_Stream(path, std::ios::binary)
		{
		}

		~FileReadStream()
		{
			m_Stream.close();
		}

		bool IsOpen() const
		{
			return m_Stream.is_open();
		}

		bool HasError() const
		{
			return m_Stream.bad() || !m_Stream.good();
		}

		uint64 Size()
		{
			return m_Stream.tellg();
		}

	public:
		void ReadBytes(void* out, uint64 size)
		{
			m_Stream.read((char*)out, size);
		}

		template<typename T>
		typename std::enable_if<std::is_trivial<T>::value>::type
			Read(T& data)
		{
			ReadBytes((void*)&data, sizeof(T));
		}

		void Read(std::string& data)
		{
			uint32 length;
			Read(length);

			data.resize(length);
			char* ptr = data.data();
			ReadBytes(ptr, length * sizeof(char));
		}
	};

	class FileWriteStream 
	{
	private:
		std::ofstream m_Stream;

	public:
		FileWriteStream(const std::filesystem::path& path)
			: m_Stream(path, std::ios::trunc | std::ios::binary | std::ios::out)
		{
		}

		~FileWriteStream()
		{
			m_Stream.close();
		}

		bool IsOpen() const
		{
			return m_Stream.is_open();
		}

	public:
		void WriteBytes(const void* data, uint64 size)
		{
			m_Stream.write((const char*)data, size);
		}

		template<typename T>
		typename std::enable_if<std::is_trivial<T>::value && !std::is_pointer<T>::value>::type
			Write(const T& data)
		{
			WriteBytes((const void*)&data, sizeof(T));
		}

		void Write(const std::string_view& data)
		{
			const uint32 length = (uint32)data.size();
			Write(length);
			WriteBytes(data.data(), length * sizeof(char));
		}
	};
}