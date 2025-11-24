#pragma once

#include <Engine/World/Asset.h>
#include <Engine/Graphics/TextureBase.h>

namespace Spikey {

	class IRHITextureCube : public IRHITexture {
	public:
		IRHITextureCube(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage)
			: IRHITexture(numMips, numMips * 6, format, usage), m_Size(size)
		{
		}

		uint32 GetSize() const { return m_Size; }

		virtual Vec3Uint GetSizeXYZ() const override { return Vec3Uint(m_Size, m_Size, 1); }
		virtual IRHITextureCube* GetTextureCube() override { return this; }
		virtual uint32 GetNumLayers() const override { return 6; }

	private:
		uint32 m_Size;
	};

	constexpr char CUBE_TEXTURE_MAGIC[4] = { 'S', 'E', 'C', 'T' };
	struct CubeTextureHeader {

		uint64 ByteSize;
		uint32 Size;
		uint32 NumMips;

		ETextureFormat Format;

		ESamplerFilter Filter : 2;
		ESamplerAddress AddressU : 2;
		ESamplerAddress AddressV : 2;
		ESamplerAddress AddressW : 2;

		uint8 _Padding[6];
	};

	class TextureCube : public IAsset {
	public:
		TextureCube(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage, UUID id);
		virtual ~TextureCube() override;

		static TRef<TextureCube> Create(BinaryReadStream& stream, UUID id);
		static TRef<TextureCube> Create(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage);

		IRHITextureCube* GetResource() { return m_RHIResource; }

		uint32 GetSize() const { return m_RHIResource->GetSize(); }
		uint32 GetNumMips() const { return m_RHIResource->GetNumMips(); }
		Vec3Uint GetSizeXYZ() const { return m_RHIResource->GetSizeXYZ(); }
		ETextureFormat GetFormat() const { return m_RHIResource->GetFormat(); }
		bool IsMipmapped() const { return m_RHIResource->IsMipmaped(); }

	private:
		TextureCubeRHIRef m_RHIResource;
	};
}