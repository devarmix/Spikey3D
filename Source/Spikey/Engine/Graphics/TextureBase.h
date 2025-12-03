#pragma once

#include <Engine/Core/RefCounted.h>
#include <Engine/Core/Math.h>

namespace Spikey {

	// forward defines
	class IRHITextureView;
	class IRHITexture;
	class IRHITexture2D;
	class IRHITextureCube;
	class IRHISamplerState;

	using TextureViewRHIRef = TRef<IRHITextureView>;
	using Texture2DRHIRef = TRef<IRHITexture2D>;
	using TextureCubeRHIRef = TRef<IRHITextureCube>;
	using TextureRHIRef = TRef<IRHITexture>;
	using SamplerStateRHIRef = TRef<IRHISamplerState>;

	enum class ETextureFormat : uint8 {
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

	// for compressed formats will return size of a single block
	uint32 TextureTexelSize(ETextureFormat format);
	uint32 NumTextureMips(uint32 width, uint32 height);
	bool IsTextureCompressed(ETextureFormat format);

	// for compressed formats will return the x and y number of blocks
	Vec2Uint TextureMipExtents(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip);
	Vec2Uint TextureMipExtents(uint32 texW, uint32 texH, uint32 mip);
	uint64 MipSizeInBytes(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip);
	uint64 TextureSizeInBytes(ETextureFormat format, uint32 width, uint32 height, uint32 numMips);

	enum class ETextureUsage : uint8 {
		None = 0,

		Sampled     = BIT(0),
		CopySrc     = BIT(1),
		CopyDst     = BIT(2),
		Storage     = BIT(3),
		ColorTarget = BIT(4),
		DepthTarget = BIT(5)
	};
	ENUM_FLAGS_OPERATORS(ETextureUsage);

	struct SubresourceRange {
		uint32 BaseMip;
		uint32 NumMips;
		uint32 BaseLayer;
		uint32 NumLayers;

		static SubresourceRange AllTexture() {
			SubresourceRange range{};
			range.BaseMip = 0;
			range.BaseLayer = 0;
			range.NumMips = ~0u;
			range.NumLayers = ~0u;

			return range;
		}
	};

	class IRHITexture : public IRefCounted {
	public:
		IRHITexture(uint32 numMips, uint32 numSubresources, ETextureFormat format, ETextureUsage usage);

		ETextureFormat GetFormat() const { return m_Format; }
		uint32 GetNumMips() const { return m_NumMips; }
		ETextureUsage GetUsage() const { return m_Usage; }

		virtual Vec3Uint GetSizeXYZ() const = 0;
		virtual uint32 GetNumLayers() const = 0;

		// dynamic cast methods
		virtual IRHITexture2D* GetTexture2D() { return nullptr; }
		virtual IRHITextureCube* GetTextureCube() { return nullptr; }
		virtual void* GetNative() const = 0;

		bool IsMipmaped() const { return GetNumMips() > 1; }
		IRHITextureView* GetView() const { return m_TextureView; }

		void Barrier(IRHICommandList* cmd, const SubresourceRange& range, EGPUAccess newAccess);

	private:
		void InitStateTracking(uint32 numSubresources);

	private:
		TextureViewRHIRef m_TextureView;

		struct TextureState {
			void SetResourceState(EGPUAccess newAccess);
			void SetSubresourceState(uint32 index, EGPUAccess newAccess);

			std::vector<EGPUAccess> SubresourceAccess;
			EGPUAccess Access;

			bool AllSubresourcesSame;
		} m_State;

		uint32 m_NumMips;
		ETextureFormat m_Format;
		ETextureUsage m_Usage;
	};

	class IRHITextureView : public IRefCounted {
	public:
		IRHITextureView(const SubresourceRange& range)
			: m_Range(range)
		{
		}

		uint32 GetNumMips() const { return m_Range.NumMips; }
		uint32 GetBaseMip() const { return m_Range.BaseMip; }
		uint32 GetBaseLayer() const { return m_Range.BaseLayer; }
		uint32 GetNumLayers() const { return m_Range.NumLayers; }

		virtual void* GetNative() const = 0;

	private:
		SubresourceRange m_Range;
	};

	enum class ESamplerFilter : uint8 {
		Point,
		Bilinear,
	    Trilinear
	};

	enum class ESamplerAddress : uint8 {
		Clamp,
		Wrap,
		Mirror
	};

	enum class ESamplerReduction : uint8 {
		Standard = 0,
		Minimum,
		Maximum
	};

	struct SamplerStateDesc {

		ESamplerFilter    Filter;
		ESamplerAddress   AddressU;
		ESamplerAddress   AddressV;
		ESamplerAddress   AddressW;
		ESamplerReduction Reduction;

		float  MipLODBias = 0.f;
		float  MinLOD = 0.f;
		float  MaxLOD = 0.f;
		uint32 MaxAnisotropy = 0;

		bool operator==(const SamplerStateDesc& other) const {
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

	class IRHISamplerState : public IRefCounted {
	public:
		IRHISamplerState() = default;
		virtual void* GetNative() const = 0;
	};
}

namespace std {
	template<>
	struct hash<Spikey::SamplerStateDesc> {
		constexpr size_t operator()(const Spikey::SamplerStateDesc& desc) const {
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