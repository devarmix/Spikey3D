#include <Engine/Graphics/TextureCube.h>
#include <Engine/Graphics/FrameRenderer.h>

namespace Spikey {

	/*
	RHITextureCube::RHITextureCube(const TextureCubeDesc& desc) : m_Desc(desc) {
		m_RHIData = 0;

		TextureViewDesc viewDesc{};
		viewDesc.BaseMip = 0;
		viewDesc.NumMips = desc.NumMips;
		viewDesc.BaseArrayLayer = 0;
		viewDesc.NumArrayLayers = 6;
		viewDesc.SourceTexture = this;

		m_TextureView = new RHITextureView(viewDesc);

		// TODO: maybe make it optional
		InitStateTracking(desc.NumMips * 6);
	}

	void RHITextureCube::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateCubeTextureRHI(m_Desc);

		if (EnumHasAllFlags(m_Desc.UsageFlags, ETextureUsage::Sampled) && m_Desc.AutoCreateSampler && !m_Desc.Sampler) {
			m_Desc.Sampler = Graphics::GetCachedSampler(m_Desc.SamplerDesc);
		}

		m_TextureView->InitRHI();
	}

	void RHITextureCube::ReleaseRHIImmediate() {
		m_TextureView->ReleaseRHIImmediate();
		delete m_TextureView;

		Graphics::GetRHI().DestroyCubeTextureRHI(m_RHIData);
	}

	void RHITextureCube::ReleaseRHI() {
		m_TextureView->ReleaseRHI();
		delete m_TextureView;

		Graphics::GetFrameRenderer().EnqueueDeferred([data = m_RHIData]() {
			Graphics::GetRHI().DestroyCubeTextureRHI(data);
			});
	} 

	void RHITextureCube::ReloadData(const TextureCubeDesc& newDesc) {
		if (!(m_Desc == newDesc)) {
			{
				Graphics::GetFrameRenderer().EnqueueDeferred([data = m_RHIData]() {
					Graphics::GetRHI().DestroyCubeTextureRHI(data);
					});
			}
			m_Desc = newDesc;
			{
				m_RHIData = Graphics::GetRHI().CreateCubeTextureRHI(m_Desc);

				if (EnumHasAllFlags(m_Desc.UsageFlags, ETextureUsage::Sampled) && m_Desc.AutoCreateSampler && !m_Desc.Sampler) {
					m_Desc.Sampler = Graphics::GetCachedSampler(m_Desc.SamplerDesc);
				}
			}

			TextureViewDesc viewDesc{};
			viewDesc.BaseMip = 0;
			viewDesc.NumMips = newDesc.NumMips;
			viewDesc.BaseArrayLayer = 0;
			viewDesc.NumArrayLayers = 6;
			viewDesc.SourceTexture = this;

			m_TextureView->ReloadData(viewDesc);
			InitStateTracking(newDesc.NumMips * 6);
		}
	} */


	TextureCube::TextureCube(const TextureCubeDesc& desc, UUID id) {
		m_ID = id;
		CreateResource(desc);
	}

	TextureCube::~TextureCube() {
		ReleaseResource();
	}

	void TextureCube::CreateResource(const TextureCubeDesc& desc) {
		m_RHIResource = new RHITextureCube(desc);
		SafeResourceInit(m_RHIResource);
	}

	void TextureCube::ReleaseResource() {
		SafeResourceRelease(m_RHIResource);
		m_RHIResource = nullptr;
	}

	TRef<TextureCube> TextureCube::Create(const TextureCubeDesc& desc) {
		return CreateRef<TextureCube>(desc, 0);
	}

	TRef<TextureCube> TextureCube::Create(BinaryReadStream& stream, UUID id) {

		char magic[4] = {};
		stream >> magic;

		if (memcmp(magic, CUBE_TEXTURE_MAGIC, sizeof(char) * 4) != 0) {
			ENGINE_ERROR("Corrupted cube texture asset file: {}", (uint64)id);
			return nullptr;
		}

		CubeTextureHeader header{};
		stream >> header;

		SamplerDesc samplDesc{};
		samplDesc.Filter = header.Filter;
		samplDesc.AddressU = header.AddressU;
		samplDesc.AddressV = header.AddressV;
		samplDesc.AddressW = header.AddressW;
		samplDesc.MaxLOD = header.NumMips;

		TextureCubeDesc desc{};
		desc.Size = header.Size;
		desc.Format = header.Format;
		desc.NumMips = header.NumMips;
		desc.UsageFlags = ETextureUsage::Sampled | ETextureUsage::CopyDst;
		desc.SamplerDesc = samplDesc;

		uint8_t* buff = new uint8_t[header.ByteSize];
		stream.ReadRaw(buff, header.ByteSize);

		TRef<TextureCube> tex = CreateRef<TextureCube>(desc, id);
		Graphics::SubmitCommand([rhi = tex->GetResource(), copySize = header.ByteSize, buff]() {
			uint64 offset = 0;

			std::vector<SubResourceCopyRegion> regions{};
			regions.reserve(rhi->GetNumMips() * 6);

			for (uint32_t m = 0; m < rhi->GetNumMips(); m++) {
				for (int f = 0; f < 6; f++) {

					SubResourceCopyRegion& region = regions.emplace_back(SubResourceCopyRegion{});
					region.ArrayLayer = f;
					region.DataOffset = offset;
					region.MipLevel = m;
					offset += MipSizeInBytes(rhi->GetFormat(), rhi->GetSizeXYZ().x, rhi->GetSizeXYZ().y, m);
				}
			}
			Graphics::GetRHI().CopyDataToTexture(buff, 0, rhi, EGPUAccess::None, EGPUAccess::SRV, regions, copySize);
			delete[] buff;
			});

		return tex;
	}
}