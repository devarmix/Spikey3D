#pragma once

#include <Engine/Resources/Asset.h>
#include <Engine/Graphics/GPUTexture.h>

namespace Spikey
{
	struct TextureHeader
	{
		uint32 Width;
		uint32 Height;
		uint32 Depth;
		uint32 MipLevels;

		PixelFormat Format;
		bool IsCubeMap;
		bool IsNormalMap;
	};

	class Texture : public AssetBase
	{
	private:
		TextureHeader m_Header;
		GPUTextureRef m_GPUTexture;

		Texture(const TextureHeader& header, Guid id);

	public:
		virtual ~Texture() override;

		uint32 Width() const
		{
			return m_Header.Width;
		}

		uint32 Height() const
		{
			return m_Header.Height;
		}

		uint32 Depth() const
		{
			return m_Header.Depth;
		}

		uint32      MipLevels() const { return m_Header.MipLevels; }
		uint32      ArraySize() const { return m_Header.IsCubeMap ? 6 : 0; }
		PixelFormat Format() const { return m_Header.Format; }
		bool        IsCubeMap() const { return m_Header.IsCubeMap; }
		bool        IsNormalMap() const { return m_Header.IsNormalMap;  }

		GPUTexture* GetGPUResource() const
		{
			return m_GPUTexture;
		}
	};

	class TextureFactory
	{
	public:
		TRefCountPtr<AssetBase> FromFile(const std::filesystem::path& path);
	};

	class GPUTextureMipUploadTask : public GPUTask
	{
	private:
		GPUTextureRef m_Texture;
		uint32 m_MipIndex;
		uint32 m_RowPitch;
		uint32 m_SlicePitch;
		std::vector<uint8> m_MipData;

	private:
		void OnResourceReleased()
		{
			m_Texture = nullptr;
			m_MipData.clear();
			m_State = State::Canceled;
		}

	public:
		GPUTextureMipUploadTask(GPUTexture* texture, uint32 mipIndex, std::vector<uint8>&& data, uint32 rowPitch, uint32 slicePitch)
			: m_Texture(texture)
			, m_MipIndex(mipIndex)
			, m_RowPitch(rowPitch)
			, m_SlicePitch(slicePitch)
		{
			assert(data.size() >= slicePitch * texture->ArraySize());
			m_MipData = std::move(data);
		}

		virtual bool OnExecute(GPUCommandContext* context) override;
	};
}