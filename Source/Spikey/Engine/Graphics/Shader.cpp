#include <Engine/Graphics/Shader.h>
#include <Engine/Graphics/GraphicsCore.h>

namespace Spikey {

	void RHIBindingSetLayout::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateBindingSetLayoutRHI(m_Desc);
	}

	void RHIBindingSetLayout::ReleaseRHI() {
		Graphics::GetRHI().DestroyBindingSetLayoutRHI(m_RHIData);
	}

	void RHIBindingSet::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateBindingSetRHI(m_Layout);
	}

	void RHIBindingSet::ReleaseRHI() {
		Graphics::GetRHI().DestroyBindingSetRHI(m_RHIData);
	}

	void RHIBindingSet::AddTextureWrite(uint32 slot, uint32 arrayEl, RHITextureView* view, EGPUAccess access) {
		m_Writes.push_back({ .Slot = slot, .ArrayElement = arrayEl, .Texture = view, .TextureAccess = access });
	}

	void RHIBindingSet::AddBufferWrite(uint32 slot, uint32 arrayEl, RHIBuffer* buffer, uint64 range, uint64 offset) {
		m_Writes.push_back({ .Slot = slot, .ArrayElement = arrayEl, .Buffer = buffer, .BufferRange = range, .BufferOffset = offset });
	}

	void RHIBindingSet::AddSamplerWrite(uint32 slot, uint32 arrayEl, RHISampler* sampler) {
		m_Writes.push_back({ .Slot = slot, .ArrayElement = arrayEl, .Sampler = sampler });
	}

	RHIShader::RHIShader(const ShaderDesc& desc, ShaderData&& data) 
		: m_Desc(desc), m_Data(std::move(data)) 
	{
		if (!m_Data.IsMaterialShader) {
			std::vector<BindingSetLayoutDesc> descs;

			descs.resize(m_Data.SetCount);
			m_Layouts.reserve(m_Data.SetCount);

			for (const auto& b : m_Data.Bindings) {
				descs[b.Set].Bindings.push_back({ .Type = b.Type, .Count = b.Count, .Slot = b.Binding });
			}

			for (const auto& desc : descs) {
				RHIBindingSetLayout* layout = new RHIBindingSetLayout(desc);
				m_Layouts.push_back(layout);
			}

			m_Data.Bindings.clear();
		}
		else {
			m_Layouts = { Graphics::GetMeshDrawLayout(), Graphics::GetMaterialLayout() };
		}
	}

    void RHIShader::InitRHI() {
		if (!m_Data.IsMaterialShader) {
			for (auto layout : m_Layouts) {
				layout->InitRHI();
			}
		}

		m_RHIData = Graphics::GetRHI().CreateShaderRHI(m_Desc, m_Data, m_Layouts);

		// clear the data after init
		m_Data.VertexData.clear();
		m_Data.PixelData.clear();
		m_Data.ComputeData.clear();
	}

	void RHIShader::ReleaseRHI() {
		Graphics::GetRHI().DestroyShaderRHI(m_RHIData);

		if (!m_Data.IsMaterialShader) {
			for (auto layout : m_Layouts) {
				layout->ReleaseRHI();
				delete layout;
			}
		}
	}

	Shader::Shader(const ShaderDesc& desc, ShaderData&& data, UUID id) {
		m_ID = id;
		CreateResource(desc, std::move(data));
	}

	Shader::~Shader() {
		ReleaseResource();
	}

	TRef<Shader> Shader::Create(BinaryReadStream& stream, UUID id) {
		char magic[4] = {};
		stream >> magic;

		if (memcmp(magic, SHADER_MAGIC, sizeof(char) * 4) != 0) {
			ENGINE_ERROR("Corrupted shader asset file: {}", (uint64)id);
			return nullptr;
		}

		ShaderData data{};
	}

	void Shader::ReleaseResource() {
		SafeResourceRelease(m_RHIResource);
	}

	void Shader::CreateResource(const ShaderDesc& desc, ShaderData&& data) {
		m_RHIResource = new RHIShader(desc, std::move(data));
		SafeResourceInit(m_RHIResource);
	}
}