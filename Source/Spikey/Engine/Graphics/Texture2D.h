#pragma once

#include <Engine/World/Asset.h>
#include <Engine/Graphics/TextureBase.h>

namespace Spikey {

	struct Texture2DDesc {

		uint32 Width;
		uint32 Height;
		uint32 NumMips = 1;
		ETextureFormat Format;
		ETextureUsage UsageFlags;

		bool AutoCreateSampler = true;

		SamplerDesc SamplerDesc;
		RHISampler* Sampler = nullptr;

		bool operator==(const Texture2DDesc& other) const {

			if (!(Width == other.Width
				&& Height == other.Height
				&& NumMips == other.NumMips
				&& Format == other.Format
				&& UsageFlags == other.UsageFlags)) return false;

			if (AutoCreateSampler) {
				if (!other.AutoCreateSampler || SamplerDesc != other.SamplerDesc) return false;
			}

			return true;
		}
	};

	constexpr char TEXTURE_2D_MAGIC[4] = { 'S', 'T', '2', 'D'};
	struct Texture2DHeader {

		uint64 ByteSize;
		uint32 Width;
		uint32 Height;
		uint32 NumMips;

		ETextureFormat Format;

		ESamplerFilter  Filter   : 2;
		ESamplerAddress AddressU : 2;
		ESamplerAddress AddressV : 2;
		ESamplerAddress AddressW : 2;

		uint8 _Padding[2];
	};

	class RHITexture2D : public IRHITexture {
	public:
		RHITexture2D(const Texture2DDesc& desc);
		virtual ~RHITexture2D() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;
		virtual void ReleaseRHIImmediate() override;

		virtual ETextureFormat GetFormat() const override { return m_Desc.Format; }
		virtual ETextureUsage GetUsage() const override { return m_Desc.UsageFlags; }
		virtual Vec3Uint GetSizeXYZ() const override { return Vec3Uint(m_Desc.Width, m_Desc.Height, 1); }
		virtual uint32 GetNumMips() const override { return m_Desc.NumMips; }
		virtual uint32 GetNumArrayLayers() const override { return 1; }
		virtual RHISampler* GetSampler() const override { return m_Desc.Sampler; }
		virtual RHITexture2D* GetTexture2D() override { return this; }

		const Texture2DDesc& GetDesc() const { return m_Desc; }

	private:
		Texture2DDesc m_Desc;
	};

	class Texture2D : public IAsset {
	public:
		Texture2D(const Texture2DDesc& desc, UUID id);
		virtual ~Texture2D() override;

		static TRef<Texture2D> Create(BinaryReadStream& stream, UUID id);

		RHITexture2D* GetResource() { return m_RHIResource; }
		void ReleaseResource();
		void CreateResource(const Texture2DDesc& desc);

		Vec3Uint GetSizeXYZ() const { return m_RHIResource->GetSizeXYZ(); }
		ETextureFormat GetFormat() const { return m_RHIResource->GetFormat(); }
		uint32 GetNumMips() const { return m_RHIResource->GetNumMips(); }
		bool IsMipmapped() const { return m_RHIResource->IsMipmaped(); }

	private:
		RHITexture2D* m_RHIResource;
	};
}