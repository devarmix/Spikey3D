#pragma once
#include <Engine/Graphics/Shader.h>

namespace Spikey {

	class Material : public IAsset {
	public:
		Material(TRef<Shader> shader, UUID id);
		virtual ~Material() = default;

		static TRef<Material> Create(BinaryReadStream& stream, UUID id);

		void SetScalar(const std::string_view& name, float value);
		void SetUint(const std::string_view& name, uint32 value);
		void SetVec2(const std::string_view& name, const Vec2& value);
		void SetVec4(const std::string_view& name, const Vec4& value);
		void SetTextureSRV(const std::string_view& name, TRef<Texture2D> value);

		float GetScalar(const std::string_view& name);
		uint32 GetUint(const std::string_view& name);
		const Vec2& GetVec2(const std::string_view& name);
		const Vec4& GetVec4(const std::string_view& name);
		TRef<Texture2D> GetTexture(const std::string_view& name);

		TRef<Shader> GetShader() const { return m_Shader; }

	private:
		TRef<Shader> m_Shader;

		// first - hash of parameter name
		std::vector<std::pair<uint64, float>> FloatCache;
		std::vector<std::pair<uint64, uint32>> UintCache;
		std::vector<std::pair<uint64, Vec2>> Vec2Cache;
		std::vector<std::pair<uint64, Vec4>> Vec4Cache;
		std::vector<std::pair<uint64, TRef<Texture2D>>> TextureCache;
	};
}