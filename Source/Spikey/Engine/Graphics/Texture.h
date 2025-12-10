#pragma once

#include <Engine/Graphics/RHIResource.h>
#include <Engine/Core/Math.h>

namespace Spikey 
{
	enum class ETextureFormat : uint8 
	{
		None = 0,
		RGBA8U,
		BGRA8U,
		RGBA16F,
		RGBA16U,
		RGBA32F,
		RGBBC1,
		RGBABC3,
		RGBABC6,
		RGBC5,
		D32F,
		R32F,
		RG16F,
		RG16U,
		RG32F,
		RG8U,
		R8U
	};

	enum class ETextureDimension : uint8
	{
		None = 0,
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		TextureCube,
		TextureCubeArray,
		Texture3D
	};

	// for compressed formats will return size of a single block
	uint32 TextureTexelSize(ETextureFormat format);
	uint32 NumTextureMips(uint32 width, uint32 height);
	bool   IsTextureCompressed(ETextureFormat format);

	// for compressed formats will return the x and y number of blocks
	Vec2Uint TextureMipExtents(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip);
	Vec2Uint TextureMipExtents(uint32 texW, uint32 texH, uint32 mip);
	uint64   MipSizeInBytes(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip);
	uint64   TextureSizeInBytes(ETextureFormat format, uint32 width, uint32 height, uint32 numMips);

	enum class ETextureFlags : uint8 
	{
		None = 0,
		Sampled     = BIT(0),
		CopySrc     = BIT(1),
		CopyDst     = BIT(2),
		Storage     = BIT(3),
		ColorTarget = BIT(4),
		DepthTarget = BIT(5)
	};
	ENUM_FLAGS_OPERATORS(ETextureFlags);

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
		uint32            Width = 1;
		uint32            Height = 1;
		uint32            Depth = 1;
		uint32            ArraySize = 1;
		uint32            MipLevels = 1;
		uint32            SampleCount = 1;
		ETextureFormat    Format = ETextureFormat::None;
		ETextureDimension Dimension = ETextureDimension::Texture2D;
		ETextureFlags     Flags = ETextureFlags::None;
		bool              EnableStateTracking = true;
	};

	struct TextureSlice
	{
		uint32 X;
		uint32 Y;
		uint32 Z;

		uint32 Width = -1;
		uint32 Height = -1;
		uint32 Depth = -1;

		uint32 MipLevel;
		uint32 ArraySlice;
	};

	struct TextureState
	{
		ERHIAccess              State;
		bool                    AllSubresourcesSame;
		std::vector<ERHIAccess> SubresourceStates;

		void Initialize(uint32 subresourceCount, ERHIAccess initialState, bool usePerStateTracking)
		{
			assert(SubresourceStates.empty() && subresourceCount > 0);
			AllSubresourcesSame = true;
			State = initialState;

			if (usePerStateTracking && subresourceCount > 1)
				SubresourceStates.resize(subresourceCount);
		}

		ERHIAccess GetSubresourceState(uint32 index)
		{
			if (AllSubresourcesSame)
				return State;
			return SubresourceStates[index];
		}

		void SetState(ERHIAccess state)
		{
			AllSubresourcesSame = true;
			State = state;
		}

		void SetSubresourceState(uint32 index, ERHIAccess state)
		{
			if (index == -1 || SubresourceStates.size() <= 1)
			{
				SetState(state);
			}
			else
			{
				assert(index < SubresourceStates.size());

				// transition for all subresources
				if (AllSubresourcesSame)
				{
					for (int32 i = 0; i < SubresourceStates.size(); i++)
						SubresourceStates[i] = State;
					AllSubresourcesSame = false;
				}

				SubresourceStates[index] = state;
			}
		}
	};

	class RHITexture : public IRHIResource
	{
	public:
		RHITexture(const TextureDesc& desc);

		uint32             GetWidth() const { return m_Desc.Width; }
		uint32             GetHeight() const { return m_Desc.Height; }
		uint32             GetDepth() const { return m_Desc.Depth; }
		uint32             GetArraySize() const { return m_Desc.ArraySize; }
		uint32             GetMipLevels() const { return m_Desc.MipLevels; }
		uint32             GetSampleCount() const { return m_Desc.SampleCount; }
		ETextureFormat     GetFormat() const { return m_Desc.Format; }
		ETextureDimension  GetDimensions() const { return m_Desc.Dimension; }
		ETextureFlags      GetFlags() const { return m_Desc.Flags; }
		TextureState&      GetState() { return m_State; }
		const TextureDesc& GetDesc() const { return m_Desc; }

	protected:
		TextureDesc  m_Desc;
		TextureState m_State;
	};

	using TextureRHIRef = TRef<RHITexture>;

	enum class ESamplerFilter : uint8
	{
	    Point,
		Bilinear,
		Trilinear
	};

	enum class ESamplerAddress : uint8 
	{
		Clamp,
		Wrap,
		Mirror
	};

	enum class ESamplerReduction : uint8 
	{
		Standard,
		Minimum,
		Maximum
	};

	struct SamplerStateDesc 
	{
		ESamplerFilter    Filter;
		ESamplerAddress   AddressU;
		ESamplerAddress   AddressV;
		ESamplerAddress   AddressW;
		ESamplerReduction Reduction;

		float  MipLODBias = 0.f;
		float  MinLOD = 0.f;
		float  MaxLOD = 0.f;
		uint32 MaxAnisotropy = 0;

		bool operator==(const SamplerStateDesc& other) const 
		{
			return (Filter == other.Filter
				&& AddressU == other.AddressU
				&& AddressV == other.AddressV
				&& AddressW == other.AddressW
				&& Reduction == other.Reduction
				&& MipLODBias == other.MipLODBias
				&& MinLOD == other.MinLOD
				&& MaxLOD == other.MaxLOD
				&& MaxAnisotropy == other.MaxAnisotropy);
		}
	};

	class RHISamplerState : public IRefCounted 
	{
	public:
		RHISamplerState(const SamplerStateDesc& desc);
		virtual void* GetNative() const = 0;

		const SamplerStateDesc& GetDesc() const 
		{
			return m_Desc;
		}

	protected:
		SamplerStateDesc m_Desc;
	};
}

namespace std 
{
	template<> struct hash<Spikey::SamplerStateDesc> 
	{
		constexpr size_t operator()(const Spikey::SamplerStateDesc& desc) const 
		{
			using namespace Spikey;

			uint64 hash = 0;
			Math::HashCombine(hash, desc.Filter);
			Math::HashCombine(hash, desc.AddressU);
			Math::HashCombine(hash, desc.AddressV);
			Math::HashCombine(hash, desc.AddressW);
			Math::HashCombine(hash, desc.Reduction);
			Math::HashCombine(hash, desc.MipLODBias);
			Math::HashCombine(hash, desc.MinLOD);
			Math::HashCombine(hash, desc.MaxLOD);
			Math::HashCombine(hash, desc.MaxAnisotropy);

			return hash;
		}
	};
}