#pragma once

#include <Utf8-Cpp/utf8.h>

namespace Spikey
{
	inline bool StringConv_Internal::UnicodeUtils::IsValidUtf8(const char* utf8Ptr)
	{
		return utf8::is_valid(utf8Ptr);
	}

	inline bool StringConv_Internal::UnicodeUtils::SkipUtf8Bom(const char*& utf8Ptr)
	{
		if (utf8::starts_with_bom(utf8Ptr, utf8Ptr + 4))
		{
			utf8Ptr += 3;
			return true;
		}

		return false;
	}

	inline bool StringConv_Internal::UnicodeUtils::SkipUtf16BomLE(const char16_t*& utf16Ptr)
	{
		if (*utf16Ptr == Utf16BomLE)
		{
			++utf16Ptr;
			return true;
		}

		return false;
	}

	inline bool StringConv_Internal::UnicodeUtils::SkipUtf16BomBE(const char16_t*& utf16Ptr)
	{
		if (*utf16Ptr == Utf16BomBE)
		{
			++utf16Ptr;
			return true;
		}

		return false;
	}

	template<typename IteratorType>
	inline uint32 StringConv_Internal::UnicodeUtils::DecodeUtf8ToUtf32(IteratorType& utf16Iterator)
	{
		return utf8::unchecked::next(utf16Iterator);
	}

	template<typename IteratorType>
	inline uint32 StringConv_Internal::UnicodeUtils::DecodeUtf16ToUtf32(IteratorType& utf16Iterator)
	{
		uint32 cp = utf8::internal::mask16(*utf16Iterator++);
		if (utf8::internal::is_lead_surrogate(cp))
		{
			uint32 trail_surrogate = utf8::internal::mask16(*utf16Iterator++);
			cp = (cp << 10) + trail_surrogate + utf8::internal::SURROGATE_OFFSET;
		}

		return cp;
	}

	template<typename IteratorType>
	inline uint32 StringConv_Internal::UnicodeUtils::DecodeWCharToUint32(IteratorType& wcharIterator)
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			return DecodeUtf16ToUtf32(wcharIterator);
		}
		else // sizeof(wchar_t) == 4
		{
			const uint32 uiResult = *wcharIterator;
			++wcharIterator;
			return uiResult;
		}
	}

	template<typename IteratorType>
	inline void StringConv_Internal::UnicodeUtils::EncodeUtf32ToUtf8(uint32 utf32, IteratorType& destIterator)
	{
		destIterator = utf8::unchecked::utf32to8(&utf32, &utf32 + 1, destIterator);
	}

	template<typename IteratorType>
	inline void StringConv_Internal::UnicodeUtils::EncodeUtf32ToUtf16(uint32 utf32, IteratorType& destIterator)
	{
		if (utf32 > 0xffff)
		{
			*destIterator++ = static_cast<uint16>((utf32 >> 10) + utf8::internal::LEAD_OFFSET);
			*destIterator++ = static_cast<uint16>((utf32 & 0x3ff) + utf8::internal::TRAIL_SURROGATE_MIN);
		}
		else
			*destIterator++ = static_cast<uint16>(utf32);
	}

	template<typename IteratorType>
	inline void StringConv_Internal::UnicodeUtils::EncodeUtf32ToWChar(uint32 utf32, IteratorType& destIterator)
	{
		if constexpr (sizeof(wchar_t) == 2)
		{
			EncodeUtf32ToUtf16(utf32, destIterator);
		}
		else
		{
			*destIterator = static_cast<wchar_t>(utf32);
			++destIterator;
		}
	}
}