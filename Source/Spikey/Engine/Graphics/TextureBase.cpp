#include <Engine/Graphics/TextureBase.h>
#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Graphics/FrameRenderer.h>

uint32 Spikey::TextureTexelSize(ETextureFormat format) {

	switch (format)
	{
	case ETextureFormat::RGBBC1:
	case ETextureFormat::RGBA16F:
	case ETextureFormat::RGBA16U:
	case ETextureFormat::RG32F:
		return 8;
	case ETextureFormat::RGBA32F:
	case ETextureFormat::RGBABC3:
	case ETextureFormat::RGBC5:
	case ETextureFormat::RGBABC6:
		return 16;
	case ETextureFormat::RGBA8U:
	case ETextureFormat::D32F:
	case ETextureFormat::R32F:
	case ETextureFormat::RG16F:
	case ETextureFormat::RG16U:
		return 4;
	case ETextureFormat::RG8U:
		return 2;
	case ETextureFormat::R8U:
		return 1;
	default:
		return 0;
	}
}

uint32 Spikey::NumTextureMips(uint32 width, uint32 height) {
	return uint32(std::floor(std::log2(std::max(width, height)))) + 1;
}

Vec2Uint Spikey::TextureMipExtents(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip) {
	bool compressed = (format == ETextureFormat::RGBBC1 || format == ETextureFormat::RGBABC6
		|| format == ETextureFormat::RGBABC3 || format == ETextureFormat::RGBC5);

	uint32 w = std::max(1u, texW >> mip);
	uint32 h = std::max(1u, texH >> mip);

	if (compressed) {
		w = (w + 3) / 4;
		h = (h + 3) / 4;
	}

	return Vec2Uint{ w, h };
}

uint64 Spikey::MipSizeInBytes(ETextureFormat format, uint32 texW, uint32 texH, uint32 mip) {
	Vec2Uint ext = TextureMipExtents(format, texW, texH, mip);
	return (uint64)ext.x * (uint64)ext.y * (uint64)TextureTexelSize(format);
}

uint64 Spikey::TextureSizeInBytes(ETextureFormat format, uint32 width, uint32 height, uint32 numMips) {
	uint64 size = 0;

	for (uint32 i = 0; i < numMips; i++) {
		size += MipSizeInBytes(format, width, height, i);
	}
	return size;
}

namespace Spikey {

	static uint32 GetSubresourceIdx(uint32 mip, uint32 layer, uint32 numMips) {
		return mip + (layer * numMips);
	}

	void TextureState::SetResourceState(EGPUAccess newAccess) {
		Access = newAccess;
		AllSubresourcesSame = true;
	}

	void TextureState::SetSubresourceState(uint32 index, EGPUAccess newAccess) {
		if (AllSubresourcesSame) {
			for (int32 i = 0; i < SubresourceAccess.size(); i++) {
				SubresourceAccess[i] = Access;
			}

			AllSubresourcesSame = false;
		}

		SubresourceAccess[index] = newAccess;
	}

	void IRHITexture::Barrier(RHICommandBuffer* cmd, uint32 baseMip, uint32 numMips, uint32 baseLayer, uint32 numLayers, EGPUAccess newAccess) {
		bool entireTexture = (numMips == GetNumMips() && numLayers == GetNumArrayLayers());

		if (entireTexture && m_State.AllSubresourcesSame) {
			bool needTransiton = (m_State.Access != newAccess) ||
				EnumHasAnyFlags(newAccess, EGPUAccess::UAVCompute | EGPUAccess::UAVGraphics);

			if (needTransiton) {
				TextureBarrierRegion region{};
				region.EntireTexture = true;
				region.LastAccess = m_State.Access;
				region.NewAccess = newAccess;

				Graphics::GetRHI().BarrierTexture(cmd, this, &region, 1);
				m_State.SetResourceState(newAccess);
			}
		}
		else {
			std::vector<TextureBarrierRegion> regions{};

			for (uint32 l = baseLayer; l < numLayers + baseLayer; l++) {
				for (uint32 m = baseMip; m < numMips + baseMip; m++) {
					uint32 subIdx = GetSubresourceIdx(m, l, GetNumMips());

					bool needTransiton = (m_State.SubresourceAccess[subIdx] != newAccess) ||
						EnumHasAnyFlags(newAccess, EGPUAccess::UAVCompute | EGPUAccess::UAVGraphics);

					if (needTransiton) {
						TextureBarrierRegion& region = regions.emplace_back(TextureBarrierRegion{});

						region.BaseArrayLayer = l;
						region.BaseMipLevel = m;
						region.LayerCount = 1;
						region.MipCount = 1;
						region.EntireTexture = false;
						region.LastAccess = m_State.SubresourceAccess[subIdx];
						region.NewAccess = newAccess;
					}

					m_State.SetSubresourceState(subIdx, newAccess);
				}
			}

			if (!regions.empty()) {
				Graphics::GetRHI().BarrierTexture(cmd, this, regions.data(), (uint32)regions.size());
			}
		}
	}

	void IRHITexture::Barrier(RHICommandBuffer* cmd, EGPUAccess newAccess) {
		Barrier(cmd, 0, GetNumMips(), 0, GetNumArrayLayers(), newAccess);
	}

	void IRHITexture::InitStateTracking(uint32 numSubresources) {
		m_State.SubresourceAccess.reserve(numSubresources);

		for (uint32 i = 0; i < numSubresources; i++) {
			m_State.SubresourceAccess.push_back(EGPUAccess::None);
		}

		m_State.Access = EGPUAccess::None;
		m_State.AllSubresourcesSame = true;
	}

	void RHITextureView::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateTextureViewRHI(m_Desc);
	}

	void RHITextureView::ReleaseRHIImmediate() {
		Graphics::GetRHI().DestroyTextureViewRHI(m_RHIData);

		if (m_MaterialIndex != ~0u) {
			Graphics::FreeShaderTextureID(m_MaterialIndex);
		}
	}

	void RHITextureView::ReleaseRHI() {
		Graphics::GetFrameRenderer().EnqueueDeferred([data = m_RHIData, matIndex = m_MaterialIndex]() {
			Graphics::GetRHI().DestroyTextureViewRHI(data);

			if (matIndex != ~0u) {
				Graphics::FreeShaderTextureID(matIndex);
			}
			});
	}

	uint32 RHITextureView::GetMaterialIndex() {
		if (m_MaterialIndex == ~0u) {
			m_MaterialIndex = Graphics::GetShaderTextureID(this);
		}

		return m_MaterialIndex;
	}

	void RHISampler::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateSamplerRHI(m_Desc);
	}

	void RHISampler::ReleaseRHI() {
		Graphics::GetRHI().DestroySamplerRHI(m_RHIData);

		if (m_MaterialIndex != ~0u) {
			Graphics::FreeShaderSamplerID(m_MaterialIndex);
		}
	}

	uint32 RHISampler::GetMaterialIndex() {
		if (m_MaterialIndex == ~0u) {
			m_MaterialIndex = Graphics::GetShaderSamplerID(this);
		}

		return m_MaterialIndex;
	}
}