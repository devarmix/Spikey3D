#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/GraphicsCore.h>

namespace Spikey {

	RHIMesh::RHIMesh(const MeshDesc& desc) : m_Desc(desc) {

		BufferDesc bufferDesc{};
		bufferDesc.UsageFlags = EBufferUsage::Storage | EBufferUsage::CopyDst | EBufferUsage::Addressable;
		bufferDesc.MemUsage = EBufferMemUsage::GPUOnly;
		bufferDesc.Size = sizeof(Vertex) * desc.Vertices.size();

		m_VertexBuffer = new RHIBuffer(bufferDesc);
		bufferDesc.Size = sizeof(uint32) * desc.Indices.size();
		m_IndexBuffer = new RHIBuffer(bufferDesc);

		// calculate bounds
		{
			m_Bounds.LowerBound = m_Desc.Vertices[0].Position;
			m_Bounds.UpperBound = m_Desc.Vertices[0].Position;

			for (int i = 1; i < m_Desc.Vertices.size(); i++) {

				Vec3 pos = Vec3(m_Desc.Vertices[i].Position);
				m_Bounds.LowerBound = glm::min(m_Bounds.LowerBound, pos);
				m_Bounds.UpperBound = glm::max(m_Bounds.UpperBound, pos);
			}
		}
	}

	void RHIMesh::InitRHI() {

		m_VertexBuffer->InitRHI();
		m_IndexBuffer->InitRHI();

		const uint64 vertexBufferSize = sizeof(Vertex) * m_Desc.Vertices.size();
		const uint64 indexBufferSize = sizeof(uint32) * m_Desc.Indices.size();

		BufferDesc stagingDesc{};
		stagingDesc.Size = vertexBufferSize + indexBufferSize;
		stagingDesc.UsageFlags = EBufferUsage::CopySrc;
		stagingDesc.MemUsage = EBufferMemUsage::CPUOnly;

		RHIBuffer* staging = new RHIBuffer(stagingDesc);
		staging->InitRHI();

		memcpy(staging->GetMappedData(), m_Desc.Vertices.data(), vertexBufferSize);
		memcpy((uint8*)staging->GetMappedData() + vertexBufferSize, m_Desc.Indices.data(), indexBufferSize);

		Graphics::GetRHI().ImmediateSubmit([&, this](RHICommandBuffer* cmd) {

			Graphics::GetRHI().CopyBuffer(cmd, staging, m_VertexBuffer, 0, 0, vertexBufferSize);
			Graphics::GetRHI().CopyBuffer(cmd, staging, m_IndexBuffer, vertexBufferSize, 0, indexBufferSize);
			});

		staging->ReleaseRHI();
		delete staging;

		if (!m_Desc.NeedCPUData) {

			m_Desc.Vertices.clear();
			m_Desc.Indices.clear();
		}
	}

	void RHIMesh::ReleaseRHI() {

		m_VertexBuffer->ReleaseRHI();
		delete m_VertexBuffer;

		m_IndexBuffer->ReleaseRHI();
		delete m_IndexBuffer;
	}

	Mesh::Mesh(const MeshDesc& desc, UUID id) {
		m_ID = id;
		CreateResource(desc);
	}

	Mesh::~Mesh() {
		ReleaseResource();
	}

	TRef<Mesh> Mesh::Create(BinaryReadStream& stream, UUID id) {
		MeshDesc desc{};

		char magic[4] = {};
		stream >> magic;

		if (memcmp(MESH_MAGIC, magic, sizeof(char) * 4) != 0) {
			ENGINE_ERROR("Mesh asset file is not valid: {}!", (uint64_t)id);
			return nullptr;
		}

		stream >> desc.Vertices >> desc.Indices >> desc.SubMeshes;
		return CreateRef<Mesh>(desc, id);
	}

	void Mesh::ReleaseResource() {
		SafeResourceRelease(m_RHIResource);
		m_RHIResource = nullptr;
	}

	void Mesh::CreateResource(const MeshDesc& desc) {
		m_RHIResource = new RHIMesh(desc);
		SafeResourceInit(m_RHIResource);
	}
}