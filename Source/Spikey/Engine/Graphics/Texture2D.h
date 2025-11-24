#pragma once

#include <Engine/World/Asset.h>
#include <Engine/Graphics/TextureBase.h>

namespace Spikey {

	class IRHITexture2D : public IRHITexture {
	public:
		IRHITexture2D(uint32 width, uint32 height, uint32 numMips, ETextureFormat format, ETextureUsage usage)
			: IRHITexture(numMips, numMips, format, usage), m_Width(width), m_Height(height)
		{
		}

		uint32 GetWidth() const { return m_Width; }
		uint32 GetHeight() const { return m_Height; }

		virtual Vec3Uint GetSizeXYZ() const override { return Vec3Uint(m_Width, m_Height, 1); }
		virtual uint32 GetNumLayers() const override { return 1; }
		virtual IRHITexture2D* GetTexture2D() override { return this; }

	private:
		uint32 m_Width;
		uint32 m_Height;
	};

	constexpr char TEXTURE_2D_MAGIC[4] = { 'S', 'T', '2', 'D' };
	struct Texture2DHeader {

		uint64 ByteSize;
		uint32 Width;
		uint32 Height;
		uint32 NumMips;

		ETextureFormat Format;

		ESamplerFilter  Filter : 2;
		ESamplerAddress AddressU : 2;
		ESamplerAddress AddressV : 2;
		ESamplerAddress AddressW : 2;

		uint8 _Padding[2];
	};

	class Texture2D : public IAsset {
	public:
		Texture2D(uint32 width, uint32 height, uint32 numMips, ETextureFormat format, ETextureUsage usage, UUID id);
		virtual ~Texture2D() override;

		static TRef<Texture2D> Create(BinaryReadStream& stream, UUID id);
		static TRef<Texture2D> Create(uint32 width, uint32 height, uint32 numMips, ETextureFormat format, ETextureUsage usage);

		IRHITexture2D* GetResource() { return m_RHIResource; }

		uint32 GetWidth() const { return m_RHIResource->GetWidth(); }
		uint32 GetHeight() const { return m_RHIResource->GetHeight(); }
		Vec3Uint GetSizeXYZ() const { return m_RHIResource->GetSizeXYZ(); }

		ETextureFormat GetFormat() const { return m_RHIResource->GetFormat(); }
		uint32 GetNumMips() const { return m_RHIResource->GetNumMips(); }
		bool IsMipmapped() const { return m_RHIResource->IsMipmaped(); }

	private:
		Texture2DRHIRef m_RHIResource;
	};
}