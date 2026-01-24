#include <Engine/Graphics/Texture.h>

namespace Spikey
{
	static uint32 FormatToSize[66] =
	{
		0,   // UNKNOWN
		1,   // R8_UINT
		1,   // R8_SINT
		1,   // R8_UNORM
		1,   // R8_SNORM
		2,   // RG8_UINT
		2,   // RG8_SINT
		2,   // RG8_UNORM
		2,   // RG8_SNORM
		2,   // R16_UINT
		2,   // R16_SINT
		2,   // R16_UNORM
		2,   // R16_SNORM
		2,   // R16_FLOAT
		2,   // BGRA4_UNORM
		2,   // B5G6R5_UNORM
		2,   // B5G5R5A1_UNORM
		4,   // RGBA8_UINT
		4,   // RGBA8_SINT
		4,   // RGBA8_UNORM
		4,   // RGBA8_SNORM
		4,   // BGRA8_UNORM
		4,   // SRGBA8_UNORM
		4,   // SBGRA8_UNORM
		4,   // R10G10B10A2_UNORM
		4,   // R11G11B10_FLOAT
		4,   // RG16_UINT
		4,   // RG16_SINT
		4,   // RG16_UNORM
		4,   // RG16_SNORM
		4,   // RG16_FLOAT
		4,   // R32_UINT
		4,   // R32_SINT
		4,   // R32_FLOAT
		8,   // RGBA16_UINT
		8,   // RGBA16_SINT
		8,   // RGBA16_FLOAT
		8,   // RGBA16_UNORM
		8,   // RGBA16_SNORM
		8,   // RG32_UINT
		8,   // RG32_SINT
		8,   // RG32_FLOAT
		12,  // RGB32_UINT
		12,  // RGB32_SINT
		12,  // RGB32_FLOAT
		16,  // RGBA32_UINT
		16,  // RGBA32_SINT
		16,  // RGBA32_FLOAT
		2,   // D16
		4,   // D24S8
		4,   // X24G8_UINT
		4,   // D32
		8,   // D32S8
		8,   // X32G8_UINT
		8,   // BC1_UNORM
		8,   // BC1_UNORM_SRGB
		16,  // BC3_UNORM
		16,  // BC3_UNORM_SRGB
		8,   // BC4_UNORM
		8,   // BC4_SNORM
		16,  // BC5_UNORM
		16,  // BC5_SNORM
		16,  // BC6H_UFLOAT
		16,  // BC6H_SFLOAT
		16,  // BC7_UNORM
		16   // BC7_UNORM_SRGB
	};

	uint32 PixelFormatInfo::SizeInBytes(PixelFormat format)
	{
		return FormatToSize[(uint8)format];
	}
	 
	bool PixelFormatInfo::HasAlpha(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::BGRA4_UNORM:
		case PixelFormat::B5G5R5A1_UNORM:
		case PixelFormat::RGBA8_UINT:
		case PixelFormat::RGBA8_SINT:
		case PixelFormat::RGBA8_UNORM:
		case PixelFormat::RGBA8_SNORM:
		case PixelFormat::BGRA8_UNORM:
		case PixelFormat::SRGBA8_UNORM:
		case PixelFormat::SBGRA8_UNORM:
		case PixelFormat::R10G10B10A2_UNORM:
		case PixelFormat::RGBA16_UINT:
		case PixelFormat::RGBA16_SINT:
		case PixelFormat::RGBA16_FLOAT:
		case PixelFormat::RGBA16_UNORM:
		case PixelFormat::RGBA16_SNORM:
		case PixelFormat::RGBA32_UINT:
		case PixelFormat::RGBA32_SINT:
		case PixelFormat::RGBA32_FLOAT:
		case PixelFormat::BC1_UNORM:
		case PixelFormat::BC1_UNORM_SRGB:
		case PixelFormat::BC3_UNORM:
		case PixelFormat::BC3_UNORM_SRGB:
		case PixelFormat::BC7_UNORM:
		case PixelFormat::BC7_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	bool PixelFormatInfo::HasDepth(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::D16:
		case PixelFormat::D24S8:
		case PixelFormat::D32:
		case PixelFormat::D32S8:
			return true;
		default:
			return false;
		}
	}

	bool PixelFormatInfo::HasStencil(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::D24S8:
		case PixelFormat::D32S8:
		case PixelFormat::X24G8_UINT:
		case PixelFormat::X32G8_UINT:
			return true;
		default:
			return false;
		}
	}

	bool PixelFormatInfo::IsCompressed(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::BC1_UNORM:
		case PixelFormat::BC1_UNORM_SRGB:
		case PixelFormat::BC3_UNORM:
		case PixelFormat::BC3_UNORM_SRGB:
		case PixelFormat::BC4_UNORM:
		case PixelFormat::BC4_SNORM:
		case PixelFormat::BC5_UNORM:
		case PixelFormat::BC5_SNORM:
		case PixelFormat::BC6H_UFLOAT:
		case PixelFormat::BC6H_SFLOAT:
		case PixelFormat::BC7_UNORM:
		case PixelFormat::BC7_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	bool PixelFormatInfo::IsSRGB(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::SRGBA8_UNORM:
		case PixelFormat::SBGRA8_UNORM:
		case PixelFormat::BC1_UNORM_SRGB:
		case PixelFormat::BC3_UNORM_SRGB:
		case PixelFormat::BC7_UNORM_SRGB:
			return true;
		default:
			return false;
		}
	}

	bool PixelFormatInfo::IsSigned(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::R8_SINT:
		case PixelFormat::R8_SNORM:
		case PixelFormat::RG8_SINT:
		case PixelFormat::RG8_SNORM:
		case PixelFormat::R16_SINT:
		case PixelFormat::R16_SNORM:
		case PixelFormat::R16_FLOAT:
		case PixelFormat::RGBA8_SINT:
		case PixelFormat::RGBA8_SNORM:
		case PixelFormat::RG16_SINT:
		case PixelFormat::RG16_SNORM:
		case PixelFormat::RG16_FLOAT:
		case PixelFormat::R32_SINT:
		case PixelFormat::R32_FLOAT:
		case PixelFormat::RGBA16_SINT:
		case PixelFormat::RGBA16_FLOAT:
		case PixelFormat::RGBA16_SNORM:
		case PixelFormat::RG32_SINT:
		case PixelFormat::RG32_FLOAT:
		case PixelFormat::RGB32_SINT:
		case PixelFormat::RGB32_FLOAT:
		case PixelFormat::RGBA32_SINT:
		case PixelFormat::RGBA32_FLOAT:
		case PixelFormat::BC4_SNORM:
		case PixelFormat::BC5_SNORM:
		case PixelFormat::BC6H_SFLOAT:
			return true;
		default:
			return false;
		}
	}

	uint32 PixelFormatInfo::BlockSize(PixelFormat format)
	{
		switch (format)
		{
		case PixelFormat::Unknown:
			return 0;
		case PixelFormat::BC1_UNORM:
		case PixelFormat::BC1_UNORM_SRGB:
		case PixelFormat::BC3_UNORM:
		case PixelFormat::BC3_UNORM_SRGB:
		case PixelFormat::BC4_UNORM:
		case PixelFormat::BC4_SNORM:
		case PixelFormat::BC5_UNORM:
		case PixelFormat::BC5_SNORM:
		case PixelFormat::BC6H_UFLOAT:
		case PixelFormat::BC6H_SFLOAT:
		case PixelFormat::BC7_UNORM:
		case PixelFormat::BC7_UNORM_SRGB:
			return 4;
		default:
			return 1;
		}
	}
}