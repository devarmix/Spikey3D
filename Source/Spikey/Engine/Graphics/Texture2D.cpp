#include <Engine/Graphics/Texture2D.h>
#include <Engine/Graphics/FrameRenderer.h>
#include <Engine/Core/Application.h>

namespace Spikey {

	RHITexture2D::RHITexture2D(const Texture2DDesc& desc) : m_Desc(desc) {
		m_RHIData = 0;

		TextureViewDesc viewDesc{};
		viewDesc.BaseMip = 0;
		viewDesc.NumMips = desc.NumMips;
		viewDesc.BaseArrayLayer = 0;
		viewDesc.NumArrayLayers = 1;
		viewDesc.SourceTexture = this;

		m_TextureView = new RHITextureView(viewDesc);

		// TODO: maybe make it optional
		InitStateTracking(desc.NumMips);
	}

	void RHITexture2D::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateTexture2DRHI(m_Desc);

		if (EnumHasAllFlags(m_Desc.UsageFlags, ETextureUsage::Sampled) && m_Desc.AutoCreateSampler && !m_Desc.Sampler) {
			m_Desc.Sampler = Graphics::GetCachedSampler(m_Desc.SamplerDesc);
		}

		m_TextureView->InitRHI();
	}

	void RHITexture2D::ReleaseRHIImmediate() {
		m_TextureView->ReleaseRHIImmediate();
		delete m_TextureView;

		Graphics::GetRHI().DestroyTexture2DRHI(m_RHIData);
	}

	void RHITexture2D::ReleaseRHI() {
		m_TextureView->ReleaseRHI();
		delete m_TextureView;

		Graphics::GetFrameRenderer().EnqueueDeferred([data = m_RHIData]() {
			Graphics::GetRHI().DestroyTexture2DRHI(data);
			});
	}

	Texture2D::Texture2D(const Texture2DDesc& desc, UUID id) {
		m_ID = id;
		CreateResource(desc);
	}

	Texture2D::~Texture2D() {
		ReleaseResource();
	}

	void Texture2D::CreateResource(const Texture2DDesc& desc) {
		m_RHIResource = new RHITexture2D(desc);
		SafeResourceInit(m_RHIResource);
	}

	void Texture2D::ReleaseResource() {
		SafeResourceRelease(m_RHIResource);
		m_RHIResource = nullptr;
	}

	TRef<Texture2D> Texture2D::Create(BinaryReadStream& stream, UUID id) {

		char magic[4] = {};
		stream >> magic;

		if (memcmp(magic, TEXTURE_2D_MAGIC, sizeof(char) * 4) != 0) {
			ENGINE_ERROR("Corrupted texture 2D asset file: {}", (uint64)id);
			return nullptr;
		}

		Texture2DHeader header{};
		stream >> header;

		SamplerDesc samplDesc{};
		samplDesc.Filter = header.Filter;
		samplDesc.AddressU = header.AddressU;
		samplDesc.AddressV = header.AddressV;
		samplDesc.AddressW = header.AddressW;
		samplDesc.MaxLOD = header.NumMips;

		Texture2DDesc desc{};
		desc.Width = header.Width;
		desc.Height = header.Height;
		desc.Format = header.Format;
		desc.NumMips = header.NumMips;
		desc.UsageFlags = ETextureUsage::Sampled | ETextureUsage::CopyDst;
		desc.SamplerDesc = samplDesc;

		uint8_t* buff = new uint8_t[header.ByteSize];
		stream.ReadRaw(buff, header.ByteSize);

		TRef<Texture2D> tex = CreateRef<Texture2D>(desc, id);
		ENQUEUE_RENDER_COMMAND(([rhi = tex->GetResource(), copySize = header.ByteSize, buff]() {
			uint64 offset = 0;

			std::vector<SubResourceCopyRegion> regions{};
			regions.reserve(rhi->GetNumMips());

			for (uint32 m = 0; m < rhi->GetNumMips(); m++) {

				SubResourceCopyRegion& region = regions.emplace_back(SubResourceCopyRegion{});
				region.ArrayLayer = 0;
				region.DataOffset = offset;
				region.MipLevel = m;
				offset += MipSizeInBytes(rhi->GetFormat(), rhi->GetSizeXYZ().x, rhi->GetSizeXYZ().y, m);
			}
			Graphics::GetRHI().CopyDataToTexture(buff, 0, rhi, EGPUAccess::None, EGPUAccess::SRV, regions, copySize);
			delete[] buff;
			}));

		return tex;
	}
}