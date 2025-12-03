#pragma once
#include <Engine/Graphics/Shader.h>

namespace Spikey {

	struct MaterialDesc {

	};

	struct MaterialParameter {
		enum class EType : uint8 {
			Uint,
			Float,
			Vec2,
			Vec4,
			Texture
		};

		union {
			bool AsBool;
			int AsInt;
			uint32 AsUint;
			float AsFloat;
			Vec2 AsVec2;
			Vec4 AsVec4;
			uint8 AsRaw[16];
		};

		TRef<Texture2D> AsTexture;

		EType ParameterType;
		uint8 BindIndex;   // index of binding for texture
		uint16 BindOffset; // offset in cb

		std::string Name;
	};

	class IMaterialBase : public IAsset {
	public:

		void SetBoolParameter(const std::string_view& name, bool value);
		bool GetBoolParameter(const std::string_view& name);

		// and so on...

		virtual IRHIPipelineState* GetPipelineState() const = 0;
		virtual const MaterialDesc& const GetDesc() = 0;
		virtual bool IsMaterialInstance() const = 0;

		const std::vector<MaterialParameter>& GetParameters() const {
			return m_Parameters;
		}

	private:
		std::vector<MaterialParameter> m_Parameters;
	};

	class Material : public IMaterialBase {
	public:

		virtual bool               IsMaterialInstance() const override { return false; }
		virtual IRHIPipelineState* GetPipelineState() const override { return m_PipelineState; }


	private:
		PipelineStateRHIRef m_PipelineState;
	};

	class MaterialInstance : public IMaterialBase {
	public:

	private:
		void OnBaseChange();
		void OnBaseParamsChange();

	private:
		TRef<Material> BaseMaterial;
	};
}