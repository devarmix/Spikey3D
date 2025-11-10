#pragma once

#include <Engine/Graphics/Texture2D.h>
#include <Engine/Graphics/Buffer.h>
#include <Engine/Graphics/TextureCube.h>

namespace Spikey {

	enum class EShaderType : uint8 {
		None = 0,

		Vertex,
		Pixel,

		// both vertex and fragment shaders in one file
		Graphics,
		Compute
	};

	enum class EShaderResourceType : uint8 {
		None = 0,

		TextureSRV,
		TextureUAV,
		BufferSRV,
		BufferUAV,
		ConstantBuffer,
		Sampler
	};

	struct BindingSetLayoutDesc {
		struct Binding {

			EShaderResourceType Type;
			uint32 Count;
			uint32 Slot;
		};

		std::vector<Binding> Bindings;
		//bool UseDescriptorIndexing = false;
	};

	class RHIBindingSetLayout : public IRHIResource {
	public:
		RHIBindingSetLayout(const BindingSetLayoutDesc& desc) : m_Desc(desc), m_RHIData(0) {}
		virtual ~RHIBindingSetLayout() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;

		RHIData GetRHIData() const { return m_RHIData; }
		const BindingSetLayoutDesc& GetDesc() const { return m_Desc; }

	private:

		RHIData m_RHIData;
		BindingSetLayoutDesc m_Desc;
	};

	struct BindingSetWriteDesc {

		uint32 Slot;
		uint32 ArrayElement;

		RHITextureView* Texture = nullptr;
		EGPUAccess TextureAccess;

		RHIBuffer* Buffer = nullptr;
		uint64 BufferRange;
		uint64 BufferOffset;

		RHISampler* Sampler = nullptr;
	};

	class RHIBindingSet : public IRHIResource {
	public:
		RHIBindingSet(RHIBindingSetLayout* layout) : m_Layout(layout), m_RHIData(0) {}
		virtual ~RHIBindingSet() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;

		void AddTextureWrite(uint32 slot, uint32 arrayEl, RHITextureView* view, EGPUAccess access);
		void AddBufferWrite(uint32 slot, uint32 arrayEl, RHIBuffer* buffer, uint64 range, uint64 offset);
		void AddSamplerWrite(uint32 slot, uint32 arrayEl, RHISampler* sampler);

		void ClearWrites() { m_Writes.clear(); }
		const std::vector<BindingSetWriteDesc>& GetWrites() const { return m_Writes; }

		RHIBindingSetLayout* GetLayout() { return m_Layout; }
		RHIData GetRHIData() const { return m_RHIData; }

	private:

		std::vector<BindingSetWriteDesc> m_Writes;
		RHIBindingSetLayout* m_Layout;
		RHIData m_RHIData;
	};

	enum class EFrontFace : uint8 {
		None = 0,

		ClockWise,
		CounterClockWise
	};

	struct ShaderDesc {

		bool EnableDepthTest = false;
		bool EnableDepthWrite = false;
		bool CullBackFaces = false;
		bool EnableAlphaBlend = false;
		bool EnableAdditiveBlend = false;

		EFrontFace FrontFace = EFrontFace::ClockWise;
		EShaderType Type;

		std::string Name;
		std::vector<ETextureFormat> RenderTargetFormats;

		bool operator==(const ShaderDesc& other) const {

			if (!(Type == other.Type
				&& Name.size() == other.Name.size()
				&& RenderTargetFormats.size() == other.RenderTargetFormats.size()
				&& EnableDepthTest == other.EnableDepthTest
				&& EnableDepthWrite == other.EnableDepthWrite
				&& CullBackFaces == other.CullBackFaces
				&& FrontFace == other.FrontFace))
				return false;

			for (uint32_t i = 0; i < RenderTargetFormats.size(); i++) {
				if (RenderTargetFormats[i] != other.RenderTargetFormats[i]) return false;
			}

			if (Name != other.Name) return false;
			return true;
		}
	};

	constexpr char SHADER_MAGIC[4] = { 'S', 'C', 'S', 'F' };

	class RHIShader : public IRHIResource {
	public:
		RHIShader(const ShaderDesc& desc);
		virtual ~RHIShader() override;

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;

		EShaderType GetShaderType() const { return m_Desc.Type; }
		const std::string& GetName() const { return m_Desc.Name; }
		bool DepthTestEnabled() const { return m_Desc.EnableDepthTest; }
		bool DepthWriteEnabled() const { return m_Desc.EnableDepthWrite; }
		bool CullBackFaces() const { return m_Desc.CullBackFaces; }
		bool AlphaBlendEnabled() const { return m_Desc.EnableAlphaBlend; }
		bool AdditiveBlendEnabled() const { return m_Desc.EnableAdditiveBlend; }

		const ShaderDesc& GetDesc() const { return m_Desc; }

		RHIData GetRHIData() const { return m_RHIData; }
		const std::vector<RHIBindingSetLayout*>& GetLayouts() const { return m_Layouts; }

	private:

		RHIData m_RHIData;
		ShaderDesc m_Desc;

		std::vector<RHIBindingSetLayout*> m_Layouts;
	};

	class Shader : public IAsset {
	public:
		Shader(const ShaderDesc& desc, UUID id);
		virtual ~Shader() override;

		static TRef<Shader> Create(BinaryReadStream& stream, UUID id);

		RHIShader* GetResource() { return m_RHIResource; }
		void ReleaseResource();
		void CreateResource(const ShaderDesc& desc);

		const std::string& GetName() const { return m_RHIResource->GetName(); }
		EShaderType GetShaderType() const { return m_RHIResource->GetShaderType(); }
		bool DepthTestEnabled() const { return m_RHIResource->DepthTestEnabled(); }
		bool DepthWriteEnabled() const { return m_RHIResource->DepthWriteEnabled(); }
		bool CullBackFaces() const { return m_RHIResource->CullBackFaces(); }
		bool AlphaBlendEnabled() const { return m_RHIResource->AlphaBlendEnabled(); }
		bool AdditiveBlendEnabled() const { return m_RHIResource->AdditiveBlendEnabled(); }

		struct MaterialData {
			std::vector<std::string> ScalarParameters;
			std::vector<std::string> UintParameters;
			std::vector<std::string> Vec2Parameters;
			std::vector<std::string> Vec4Parameters;
			std::vector<std::string> TextureParameters;
		};

		const MaterialData& GetMaterialData() const { return m_MaterialData; }

	private:
		RHIShader* m_RHIResource;
		MaterialData m_MaterialData;
	};
}