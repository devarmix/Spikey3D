#pragma once

#include <Engine/Core/Common.h>
#include <Engine/Core/RefCounted.h>
#include <Engine/Graphics/GraphicsTypes.h>

namespace Spikey
{
	enum class PixelFormat : uint8
	{
		Unknown,

		R8_UINT,
		R8_SINT,
		R8_UNORM,
		R8_SNORM,
		RG8_UINT,
		RG8_SINT,
		RG8_UNORM,
		RG8_SNORM,
		R16_UINT,
		R16_SINT,
		R16_UNORM,
		R16_SNORM,
		R16_FLOAT,
		BGRA4_UNORM,
		B5G6R5_UNORM,
		B5G5R5A1_UNORM,
		RGBA8_UINT,
		RGBA8_SINT,
		RGBA8_UNORM,
		RGBA8_SNORM,
		BGRA8_UNORM,
		SRGBA8_UNORM,
		SBGRA8_UNORM,
		R10G10B10A2_UNORM,
		R11G11B10_FLOAT,
		RG16_UINT,
		RG16_SINT,
		RG16_UNORM,
		RG16_SNORM,
		RG16_FLOAT,
		R32_UINT,
		R32_SINT,
		R32_FLOAT,
		RGBA16_UINT,
		RGBA16_SINT,
		RGBA16_FLOAT,
		RGBA16_UNORM,
		RGBA16_SNORM,
		RG32_UINT,
		RG32_SINT,
		RG32_FLOAT,
		RGB32_UINT,
		RGB32_SINT,
		RGB32_FLOAT,
		RGBA32_UINT,
		RGBA32_SINT,
		RGBA32_FLOAT,

		D16,
		D24S8,
		X24G8_UINT,
		D32,
		D32S8,
		X32G8_UINT,

		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_UNORM,
		BC4_SNORM,
		BC5_UNORM,
		BC5_SNORM,
		BC6H_UFLOAT,
		BC6H_SFLOAT,
		BC7_UNORM,
		BC7_UNORM_SRGB,

		MAX,
	};

	struct PixelFormatInfo
	{
		static uint32 SizeInBytes(PixelFormat format);
		static uint32 BlockSize(PixelFormat format);

		static bool HasAlpha(PixelFormat format);
		static bool HasDepth(PixelFormat format);
		static bool HasStencil(PixelFormat format);
		static bool IsCompressed(PixelFormat format);
		static bool IsSRGB(PixelFormat format);
		static bool IsSigned(PixelFormat format);
	};

	enum class TextureDimension : uint8
	{
		Unknown = 0,
		Texture2D,
		TextureCube,
		Texture3D
	};

	enum class TextureFlags : uint8
	{
		None = 0,
		ShaderResource = BIT(0),
		UnorderedAccess = BIT(1),
		RenderTarget = BIT(2),
		DepthStencil = BIT(3),

		PerMipViews = BIT(4),
		PerSliceViews = BIT(5),
		StreamedTexture = BIT(6)
	};
	ENUM_FLAGS_OPERATORS(TextureFlags);

	struct TextureSubresourceSet
	{
		uint32 BaseMip;
		uint32 NumMips;
		uint32 BaseLayer;
		uint32 NumLayers;

		static TextureSubresourceSet AllTexture()
		{
			TextureSubresourceSet range{};
			range.BaseMip = 0;
			range.BaseLayer = 0;
			range.NumMips = ~0u;
			range.NumLayers = ~0u;

			return range;
		}
	};

	struct TextureDesc
	{
		uint32           Width = 1;
		uint32           Height = 1;
		uint32           Depth = 1;
		uint32           ArraySize = 1;
		uint32           MipLevels = 1;
		uint32           SampleCount = 1;
		PixelFormat      Format = PixelFormat::Unknown;
		TextureDimension Dimension = TextureDimension::Texture2D;
		TextureFlags     Flags = TextureFlags::None;
	};

	class GPUTextureView
	{
	protected:
		GPUTextureView() = default;

		GPUTexture* m_Owner = nullptr;
		PixelFormat m_Format = PixelFormat::Unknown;

		void Init(GPUTexture* owner, PixelFormat format)
		{
			m_Owner = owner;
			m_Format = format;
		}

	public:
		PixelFormat GetFormat() const
		{
			return m_Format;
		}

		GPUTexture* GetOwner() const
		{
			return m_Owner;
		}
	};

	class GPUTexture : public RefCounted
	{
	protected:
		GPUTexture(const TextureDesc& desc) 
			: m_Desc(desc)
		{
		}

		TextureDesc m_Desc;

	public:
		virtual void*           GetNative() const = 0;
		virtual GPUTextureView* View(int32 arrayIndex) const = 0;
		virtual GPUTextureView* View(int32 arrayIndex, int32 mipLevel) const = 0;
		virtual GPUTextureView* ViewArray() const = 0;
		virtual GPUTextureView* ViewVolume() const = 0;
		virtual GPUTextureView* ViewReadOnlyDepth() const = 0;
		virtual GPUTextureView* ViewStencil() const = 0;
		        GPUTextureView* View() const { return View(0); }

		uint32 Width() const { return m_Desc.Width; }
		uint32 Height() const { return m_Desc.Height; }
		uint32 Depth() const { return m_Desc.Depth; }
		uint32 ArraySize() const { return m_Desc.ArraySize; }
		uint32 MipLevels() const { return m_Desc.MipLevels; }
		uint32 SampleCount() const { return m_Desc.SampleCount; }
		PixelFormat Format() const { return m_Desc.Format; }
		TextureDimension Dimension() const { return m_Desc.Dimension; }
		TextureFlags Flags() const { return m_Desc.Flags; }

		const TextureDesc& GetDesc() const 
		{ 
			return m_Desc; 
		}
	};

	enum class SamplerFilter : uint8
	{
	    Point,
		Bilinear,
		Trilinear,
		Anisotropic
	};

	enum class SamplerAddressMode : uint8 
	{
		Wrap,
		Clamp,
		Mirror
	};

	struct SamplerStateDesc 
	{
		SamplerFilter      Filter;
		SamplerAddressMode AddressU;
		SamplerAddressMode AddressV;
		SamplerAddressMode AddressW;

		float MipBias = 0.f;
		float MinMipLevel = 0.f;
		float MaxMipLevel = 0.f;
		uint32 MaxAnisotropy = 0;

		bool operator==(const SamplerStateDesc& other) const 
		{
			return (Filter == other.Filter
				&& AddressU == other.AddressU
				&& AddressV == other.AddressV
				&& AddressW == other.AddressW
				&& MipBias == other.MipBias
				&& MinMipLevel == other.MinMipLevel
				&& MaxMipLevel == other.MaxMipLevel);
		}
	};

	class GPUSamplerState : public RefCounted
	{
	protected:
		GPUSamplerState(const SamplerStateDesc& desc)
			: m_Desc(desc)
		{
		}

		SamplerStateDesc m_Desc;

	public:
		virtual void* GetNative() const = 0;

		const SamplerStateDesc& GetDesc() const 
		{
			return m_Desc;
		}
	};
}

namespace std 
{
	template<> struct hash<Spikey::SamplerStateDesc> 
	{
		constexpr size_t operator()(const Spikey::SamplerStateDesc& desc) const 
		{
			uint64 hash = 0;
			HashCombine(hash, desc.Filter);
			HashCombine(hash, desc.AddressU);
			HashCombine(hash, desc.AddressV);
			HashCombine(hash, desc.AddressW);
			HashCombine(hash, desc.MipBias);
			HashCombine(hash, desc.MinMipLevel);
			HashCombine(hash, desc.MaxMipLevel);
			HashCombine(hash, desc.MaxAnisotropy);

			return hash;
		}
	};
}