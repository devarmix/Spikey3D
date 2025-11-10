#pragma once

#include <Engine/World/Asset.h>
#include <Engine/Graphics/TextureBase.h>

namespace Spikey {

	struct TextureCubeDesc {

		uint32 Size;
		uint32 NumMips = 1;

		ETextureFormat Format;
		ETextureUsage UsageFlags;
		bool AutoCreateSampler = true;

		SamplerDesc SamplerDesc;
		RHISampler* Sampler = nullptr;

		bool operator==(const TextureCubeDesc& other) const {

			if (!(Size == other.Size
				&& NumMips == other.NumMips
				&& Format == other.Format
				&& UsageFlags == other.UsageFlags)) return false;

			if (AutoCreateSampler) {
				if (!other.AutoCreateSampler || SamplerDesc != other.SamplerDesc) return false;
			}

			return true;
		}
	};

	constexpr char CUBE_TEXTURE_MAGIC[4] = { 'S', 'E', 'C', 'T' };
	struct CubeTextureHeader {

		uint64 ByteSize;
		uint32 Size;
		uint32 NumMips;

		ETextureFormat Format;

		ESamplerFilter Filter    : 2;
		ESamplerAddress AddressU : 2;
		ESamplerAddress AddressV : 2;
		ESamplerAddress AddressW : 2;

		uint8 _Padding[6];
	};

	class RHITextureCube : public IRHITexture {
	public:
		RHITextureCube(const TextureCubeDesc& desc);
		virtual ~RHITextureCube() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;
		virtual void ReleaseRHIImmediate() override;

		virtual ETextureFormat GetFormat() const override { return m_Desc.Format; }
		virtual ETextureUsage GetUsage() const override { return m_Desc.UsageFlags; }
		virtual Vec3Uint GetSizeXYZ() const override { return Vec3(m_Desc.Size, m_Desc.Size, 1); }
		virtual uint32_t GetNumMips() const override { return m_Desc.NumMips; }
		virtual uint32 GetNumArrayLayers() const override { return 6; }
		virtual RHISampler* GetSampler() const override { return m_Desc.Sampler; }
		virtual RHITextureCube* GetTextureCube() override { return this; }

		const TextureCubeDesc& GetDesc() { return m_Desc; }

	private:
		TextureCubeDesc m_Desc;
	};

	class TextureCube : public IAsset {
	public:
		TextureCube(const TextureCubeDesc& desc, UUID id);
		virtual ~TextureCube() override;

		static TRef<TextureCube> Create(BinaryReadStream& stream, UUID id);

		RHITextureCube* GetResource() { return m_RHIResource; }
		void ReleaseResource();
		void CreateResource(const TextureCubeDesc& desc);

		Vec3Uint GetSizeXYZ() const { return m_RHIResource->GetSizeXYZ(); }
		ETextureFormat GetFormat() const { return m_RHIResource->GetFormat(); }
		uint32_t GetNumMips() const { return m_RHIResource->GetNumMips(); }
		bool IsMipmapped() const { return m_RHIResource->IsMipmaped(); }

	private:
		RHITextureCube* m_RHIResource;
	};
}