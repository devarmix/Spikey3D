#pragma once

#include <Engine/Core/Common.h>

namespace Spikey
{
	template<typename CharType, int32 InlinedSize = 512>
	class StringAsBase
	{
	protected:
		int32 m_Length = 0;
		CharType* m_Dynamic = nullptr;
		CharType m_Inlined[InlinedSize] = {};

		StringAsBase()
			: m_Length(0)
		{
			m_Inlined[0] = '\0';
		}

	public:
		~StringAsBase()
		{
			if (m_Dynamic)
			{
				free(m_Dynamic);
				m_Dynamic = nullptr;
			}
		}

	public:
		const CharType* Get() const
		{
			return m_Dynamic ? m_Dynamic : m_Inlined;
		}

		operator const CharType* () const
		{
			return Get();
		}

		int32 Length() const
		{
			return m_Length;
		}

	protected:
		CharType* GetDataPtr(int32 length)
		{
			m_Length = length;
			if (length + 1 < InlinedSize)
			{
				return m_Inlined;
			}
			else
			{
				if (m_Dynamic == nullptr)
				{
					m_Dynamic = (CharType*)malloc(length * sizeof(CharType));
				}
				return m_Dynamic;
			}
		}
	};

	namespace StringConv_Internal
	{
		class CountingIterator
		{
		private:
			int32 m_Counter;

		public:
			CountingIterator()
				: m_Counter(0)
			{
			}

			const CountingIterator& operator* () const { return *this; }
			const CountingIterator& operator++() { ++m_Counter; return *this; }
			const CountingIterator& operator++(int32) { ++m_Counter; return *this; }
			const CountingIterator& operator+=(const int32 Amount) { m_Counter += Amount; return *this; }

			template <typename T>
			T operator=(T Val) const
			{
				return Val;
			}

			friend int32 operator-(CountingIterator lhs, CountingIterator rhs)
			{
				return lhs.m_Counter - rhs.m_Counter;
			}

			int32 GetCount() const
			{
				return m_Counter;
			}
		};

		class UnicodeUtils
		{
		public:
			static constexpr uint16 Utf16BomLE = 0xfeff;
			static constexpr uint16 Utf16BomBE = 0xfffe;

			static bool IsValidUtf8(const char* utf8Ptr);
			static bool SkipUtf8Bom(const char*& utf8Ptr);
			static bool SkipUtf16BomLE(const char16_t*& utf16Ptr);
			static bool SkipUtf16BomBE(const char16_t*& utf16Ptr);

			template<typename IteratorType>
			static uint32 DecodeUtf8ToUtf32(IteratorType& utf16Iterator);

			template<typename IteratorType>
			static uint32 DecodeUtf16ToUtf32(IteratorType& utf16Iterator);

			template<typename IteratorType>
			static uint32 DecodeWCharToUint32(IteratorType& wcharIterator);

			template<typename IteratorType>
			static void EncodeUtf32ToUtf8(uint32 utf32, IteratorType& destIterator);

			template<typename IteratorType>
			static void EncodeUtf32ToUtf16(uint32 utf32, IteratorType& destIterator);

			template<typename IteratorType>
			static void EncodeUtf32ToWChar(uint32 utf32, IteratorType& destIterator);
		};
	}

	template<int32 InlinedSize = 512>
	class StringAsUtf8 : public StringAsBase<char, InlinedSize>
	{
	public:
		typedef char CharType;
		typedef StringAsBase<CharType, InlinedSize> Base;

	public:
		StringAsUtf8()
		{
		}

		StringAsUtf8(const char16_t* text)
		{
			*this = text;
		}

		StringAsUtf8(const wchar_t* text)
		{
			*this = text;
		}

		void operator=(const char16_t* text)
		{
			assert(text && "Input data is not a valid utf16 string!");

			StringConv_Internal::CountingIterator utf8Length{};
			FromUtf16(text, utf8Length);

			CharType* dataPtr = Base::GetDataPtr(utf8Length.GetCount());
			FromUtf16(text, dataPtr);
		}

		void operator=(const wchar_t* text)
		{
			assert(text && "Input data is not a valid wchar string!");

			StringConv_Internal::CountingIterator utf8Length{};
			FromWchar(text, utf8Length);

			CharType* dataPtr = Base::GetDataPtr(utf8Length.GetCount());
			FromWchar(text, dataPtr);
		}

	private:
		template<typename IteratorType>
		void FromUtf16(const char16_t* src, IteratorType& destIterator)
		{
			StringConv_Internal::UnicodeUtils::SkipUtf16BomLE(src);
			assert(!StringConv_Internal::UnicodeUtils::SkipUtf16BomBE(src) && "Utf16 Big Endian is currently not supported.");

			while (*src != '\0')
			{
				const uint32 utf32 = StringConv_Internal::UnicodeUtils::DecodeUtf16ToUtf32(src);
				StringConv_Internal::UnicodeUtils::EncodeUtf32ToUtf8(utf32, destIterator);
			}

			*destIterator = '\0';
		}

		template<typename IteratorType>
		void FromWchar(const wchar_t* src, IteratorType& destIterator)
		{
			while (*src != '\0')
			{
				const uint32 utf32 = StringConv_Internal::UnicodeUtils::DecodeWCharToUint32(src);
				StringConv_Internal::UnicodeUtils::EncodeUtf32ToUtf8(utf32, destIterator);
			}

			*destIterator = '\0';
		}
	};

	template<int32 InlinedSize = 512>
	class StringAsUtf16 : public StringAsBase<char16_t, InlinedSize>
	{
	public:
		typedef char16_t CharType;
		typedef StringAsBase<CharType, InlinedSize> Base;

	public:
		StringAsUtf16()
		{
		}

		StringAsUtf16(const char* text)
		{
			*this = text;
		}

		StringAsUtf16(const wchar_t* text)
		{
			*this = text;
		}

		void operator=(const char* text)
		{
			assert(text && "Input data is not a valid utf8 string!");

			StringConv_Internal::CountingIterator utf16Length{};
			FromUtf8(text, utf16Length);

			CharType* dataPtr = Base::GetDataPtr(utf16Length.GetCount());
			FromUtf8(text, dataPtr);
		}

		void operator=(const wchar_t* text)
		{
			assert(text && "Input data is not a valid wchar string!");

			StringConv_Internal::CountingIterator utf16Length{};
			FromWchar(text, utf16Length);

			CharType* dataPtr = Base::GetDataPtr(utf16Length.GetCount());
			FromWchar(text, dataPtr);
		}

	private:
		template<typename IteratorType>
		void FromUtf8(const char* src, IteratorType& destIterator)
		{
#if _DEBUG
			assert(StringConv_Internal::UnicodeUtils::IsValidUtf8(src) && "Input data is not a valid utf8 string!");
#endif

			StringConv_Internal::UnicodeUtils::SkipUtf8Bom(src);
			while (*src != '\0')
			{
				const uint32 utf32 = StringConv_Internal::UnicodeUtils::DecodeUtf8ToUtf32(src);
				StringConv_Internal::UnicodeUtils::EncodeUtf32ToUtf16(utf32, destIterator);
			}

			*destIterator = '\0';
		}

		template<typename IteratorType>
		void FromWchar(const wchar_t* src, IteratorType& destIterator)
		{
			while (*src != '\0')
			{
				const uint32 utf32 = StringConv_Internal::UnicodeUtils::DecodeWCharToUint32(src);
				StringConv_Internal::UnicodeUtils::EncodeUtf32ToUtf16(utf32, destIterator);
			}

			*destIterator = '\0';
		}
	};

	template<int32 InlinedSize = 512>
	class StringAsWChar : public StringAsBase<wchar_t, InlinedSize>
	{
	public:
		typedef wchar_t CharType;
		typedef StringAsBase<CharType, InlinedSize> Base;

	public:
		StringAsWChar()
		{
		}

		StringAsWChar(const char* text)
		{
			*this = text;
		}

		StringAsWChar(const char16_t* text)
		{
			*this = text;
		}

		void operator=(const char* text)
		{
			assert(text && "Input data is not a valid utf8 string!");

			StringConv_Internal::CountingIterator wcharLength{};
			FromUtf8(text, wcharLength);

			CharType* dataPtr = Base::GetDataPtr(wcharLength.GetCount());
			FromUtf8(text, dataPtr);
		}

		void operator=(const char16_t* text)
		{
			assert(text && "Input data is not a valid utf16 string!");

			StringConv_Internal::CountingIterator wcharLength{};
			FromUtf16(text, wcharLength);

			CharType* dataPtr = Base::GetDataPtr(wcharLength.GetCount());
			FromUtf16(text, dataPtr);
		}

	private:
		template<typename IteratorType>
		void FromUtf8(const char* src, IteratorType& destIterator)
		{
#if _DEBUG
			assert(StringConv_Internal::UnicodeUtils::IsValidUtf8(src) && "Input data is not a valid utf8 string!");
#endif

			StringConv_Internal::UnicodeUtils::SkipUtf8Bom(src);
			while (*src != '\0')
			{
				const uint32 utf32 = StringConv_Internal::UnicodeUtils::DecodeUtf8ToUtf32(src);
				StringConv_Internal::UnicodeUtils::EncodeUtf32ToWChar(utf32, destIterator);
			}

			*destIterator = '\0';
		}

		template<typename IteratorType>
		void FromUtf16(const char16_t* src, IteratorType& destIterator)
		{
			StringConv_Internal::UnicodeUtils::SkipUtf16BomLE(src);
			assert(!StringConv_Internal::UnicodeUtils::SkipUtf16BomBE(src) && "Utf16 Big Endian is currently not supported.");

			while (*src != '\0')
			{
				const uint32 utf32 = StringConv_Internal::UnicodeUtils::DecodeUtf16ToUtf32(src);
				StringConv_Internal::UnicodeUtils::EncodeUtf32ToWchar(utf32, destIterator);
			}

			*destIterator = '\0';
		}
	};
}

#include <Engine/Core/StringConversions_inl.h>