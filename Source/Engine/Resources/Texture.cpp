#include <Engine/Resources/Texture.h>
#include <Engine/Core/Engine.h>
#include <Engine/Serialization/BinaryStream.h>

namespace Spikey
{
	Texture::Texture(const TextureHeader& header, Guid id)
		: AssetBase(id)
		, m_Header(header)
	{
		TextureDesc desc
		{
			.Width = header.Width,
			.Height = header.Height,
			.Depth = header.Depth,
			.ArraySize = 1,
			.MipLevels = header.MipLevels,
			.Format = header.Format,
			.Dimension = TextureDimension::Texture2D
		};

		if (header.IsCubeMap)
		{
			desc.ArraySize = 6;
			desc.Dimension = TextureDimension::TextureCube;
		}

		m_GPUTexture = Engine::GraphicsDevice->CreateTexture(desc);
	}

	Texture::~Texture()
	{
	}

	bool GPUTextureMipUploadTask::OnExecute(GPUCommandContext* context)
	{
		if (!m_Texture)
			return false;

		const uint8* data = m_MipData.data();
		const uint32 arraySize = m_Texture->ArraySize();

		for (uint32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			context->UpdateTexture(m_Texture, arrayIndex, m_MipIndex, data, m_RowPitch, m_SlicePitch);
			data += m_SlicePitch;
		}

		return true;
	}

	static char TEXTURE_MAGIC_ID[4] = { 'S', 'B', 'T', 'F' };

	TRefCountPtr<AssetBase> TextureFactory::FromFile(const std::filesystem::path& path)
	{
		BinaryReadStream file(path);
		if (!file.IsOpen())
		{
			ENGINE_ERROR("Failed to find texture file at: {}", path.string());
			return nullptr;
		}

		uint32 magicID = 0;
		file >> magicID;

		if (memcmp(&magicID, TEXTURE_MAGIC_ID, sizeof(uint32)) != 0)
		{
			ENGINE_ERROR("Invalid texture file at: {}", path.string());
			return nullptr;
		}

		TextureHeader header = {};
		file >> header;
	}
}