#pragma once

#include <Engine/Graphics/Resource.h>
#include <Engine/Utils/MathUtils.h>

namespace Spikey {

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

	// for compressed formats will return size of single block
	uint32 TextureTexelSize(ETextureFormat format);
	uint32 NumTextureMips(uint32 width, uint32 height);

	// for compressed formats will return the x and y number of blocks
	Vec2Uint TextureMipExtents(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip);
	uint64 MipSizeInBytes(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip);
	uint64 TextureSizeInBytes(ETextureFormat format, uint32 width, uint32 height, uint32 numMips);

	enum class ETextureUsage : uint8 {
		None = 0,

		Sampled = BIT(0),
		CopySrc = BIT(1),
		CopyDst = BIT(2),
		Storage = BIT(3),
		ColorTarget = BIT(4),
		DepthTarget = BIT(5)
	};
	ENUM_FLAGS_OPERATORS(ETextureUsage);

	class RHITextureView;
	class RHISampler;
	class RHITexture2D;
	class RHITextureCube;
	class RHICommandBuffer;

	struct TextureState {
		void SetResourceState(EGPUAccess newAccess);
		void SetSubresourceState(uint32 index, EGPUAccess newAccess);

		std::vector<EGPUAccess> SubresourceAccess;
		EGPUAccess Access;

		bool AllSubresourcesSame;
	};

	class IRHITexture : public IRHIResource {
	public:
		virtual ETextureFormat GetFormat() const = 0;
		virtual ETextureUsage GetUsage() const = 0;
		virtual Vec3Uint GetSizeXYZ() const = 0;
		virtual uint32 GetNumMips() const = 0;
		virtual uint32 GetNumArrayLayers() const = 0;
		virtual RHISampler* GetSampler() const = 0;

		// dynamic cast methods
		virtual RHITexture2D* GetTexture2D() { return nullptr; }
		virtual RHITextureCube* GetTextureCube() { return nullptr; }

		bool IsMipmaped() const { return GetNumMips() > 1; }

		RHIData GetRHIData() const { return m_RHIData; }
		RHITextureView* GetView() const { return m_TextureView; }

		void Barrier(RHICommandBuffer* cmd, uint32 baseMip, uint32 numMips, uint32 baseLayer, uint32 numLayers, EGPUAccess newAccess);
		void Barrier(RHICommandBuffer* cmd, EGPUAccess newAccess);

	protected:
		void InitStateTracking(uint32 numSubresources);

	protected:
		RHIData m_RHIData;
		RHITextureView* m_TextureView;

	private:
		TextureState m_State;
	};

	struct TextureViewDesc {

		uint32 BaseMip;
		uint32 NumMips;
		uint32 BaseArrayLayer;
		uint32 NumArrayLayers;

		IRHITexture* SourceTexture;

		bool operator==(const TextureViewDesc& other) const {

			return (BaseMip == other.BaseMip
				&& BaseArrayLayer == other.BaseArrayLayer
				&& NumMips == other.NumMips
				&& NumArrayLayers == other.NumArrayLayers
				&& SourceTexture == other.SourceTexture);
		}
	};

	class RHITextureView : public IRHIResource {
	public:
		RHITextureView(const TextureViewDesc& desc) : m_Desc(desc), m_RHIData(0), m_MaterialIndex(~0u) {}
		virtual ~RHITextureView() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;
		virtual void ReleaseRHIImmediate() override;

		RHIData GetRHIData() const { return m_RHIData; }

		uint32 GetNumMips() const { return m_Desc.NumMips; }
		uint32 GetBaseMip() const { return m_Desc.BaseMip; }
		uint32 GetBaseArrayLayer() const { return m_Desc.BaseArrayLayer; }
		uint32 GetNumArrayLayers() const { return m_Desc.NumArrayLayers; }

		IRHITexture* GetSourceTexture() const { return m_Desc.SourceTexture; }

		const TextureViewDesc& GetDesc() const { return m_Desc; }
		uint32 GetMaterialIndex();

	private:

		RHIData m_RHIData;
		TextureViewDesc m_Desc;

		uint32 m_MaterialIndex;
	};

	enum class ESamplerFilter : uint8 {
		None = 0,

		Point,
		Bilinear,
	    Trilinear
	};

	enum class ESamplerAddress : uint8 {
		None = 0,

		Repeat,
		Clamp,
		Mirror
	};

	enum class ESamplerReduction : uint8 {
		None = 0,

		Minimum,
		Maximum
	};

	struct SamplerDesc {

		ESamplerFilter Filter;

		ESamplerAddress AddressU;
		ESamplerAddress AddressV;
		ESamplerAddress AddressW;

		ESamplerReduction Reduction = ESamplerReduction::None;

		float MipLODBias = 0.f;
		float MinLOD = 0.f;
		float MaxLOD = 0.f;
		float MaxAnisotropy = 0.f;

		bool operator==(const SamplerDesc& other) const {

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

	class RHISampler : public IRHIResource {
	public:
		RHISampler(const SamplerDesc& desc) : m_Desc(desc), m_RHIData(0), m_MaterialIndex(~0u) {}
		virtual ~RHISampler() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;

		const SamplerDesc& GetDesc() const { return m_Desc; }
		RHIData GetRHIData() const { return m_RHIData; }

		uint32 GetMaterialIndex();

	private:

		SamplerDesc m_Desc;
		RHIData m_RHIData;

		uint32 m_MaterialIndex;
	};
}