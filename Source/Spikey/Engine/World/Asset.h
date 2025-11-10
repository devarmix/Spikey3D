#pragma once

#include <Engine/Core/Common.h>
#include <Engine/Utils/Misc.h>
#include <Engine/Serialization/BinaryStream.h>

namespace Spikey {

	class UUID {
	public:
		UUID();
		UUID(uint64 id);
		UUID(const UUID&) = default;

		static UUID Generate();
		operator uint64() const { return m_ID; }

		struct Hasher {
			size_t operator()(const UUID& id) const {
				return (uint64)id;
			}
		};

	private:
		uint64 m_ID;
	};

	enum class EAssetType : uint8 {
		None = 0,

		Texture2D,
		CubeTexture,
		Material,
		Mesh,
		World
	};

	class IAsset : public IRefCounted {
	public:
		virtual ~IAsset() = default;

		UUID GetUUID() const { return m_ID; }
		virtual void AddRef() const override final;
		virtual void Release() const override final;

	protected:
		UUID m_ID;
	};

	class IAssetRegistry {
	public:
		virtual ~IAssetRegistry() = default;

		virtual TRef<IAsset> LoadAsset(UUID id) = 0;
		virtual void UnloadAsset(UUID id) = 0;
		virtual void Save() = 0;
		virtual void Deserialize() = 0;
	};
}