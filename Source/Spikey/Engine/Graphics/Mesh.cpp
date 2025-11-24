#include <Engine/Graphics/Mesh.h>
#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Core/Application.h>

void Spikey::CalculateMeshBounds(const std::vector<Vertex>& vertices, AABBBounds& out) {
	out.LowerBound = vertices[0].Position;
	out.UpperBound = vertices[0].Position;

	for (int i = 1; i < vertices.size(); i++) {
		Vec3 pos = Vec3(vertices[i].Position);

		out.LowerBound = glm::min(out.LowerBound, pos);
		out.UpperBound = glm::max(out.UpperBound, pos);
	}
}

namespace Spikey {

    Mesh::Mesh(MeshData&& data, AABBBounds* bounds, UUID id) {
		m_ID = id;
		m_SubMeshes = std::move(data.SubMeshes);

		if (bounds) {
			m_Bounds = *bounds;
		}
		else {
			CalculateMeshBounds(data.Vertices, m_Bounds);
		}

		BufferDesc bufferDesc{};
		bufferDesc.UsageFlags = EBufferUsage::Storage | EBufferUsage::CopyDst | EBufferUsage::Addressable;
		bufferDesc.MemUsage = EBufferMemUsage::GPUOnly;
		bufferDesc.Size = sizeof(Vertex) * data.Vertices.size();

		m_VertexBuffer = new RHIBuffer(bufferDesc);
		bufferDesc.Size = sizeof(uint32) * data.Indices.size();
		m_IndexBuffer = new RHIBuffer(bufferDesc);

		Graphics::SubmitCommand([vb = m_VertexBuffer, ib = m_IndexBuffer, dt = std::move(data)]() {
			vb->InitRHI();
			ib->InitRHI();

			const uint64 vertexBufferSize = sizeof(Vertex) * dt.Vertices.size();
			const uint64 indexBufferSize = sizeof(uint32) * dt.Indices.size();

			BufferDesc stagingDesc{};
			stagingDesc.Size = vertexBufferSize + indexBufferSize;
			stagingDesc.UsageFlags = EBufferUsage::CopySrc;
			stagingDesc.MemUsage = EBufferMemUsage::CPUOnly;

			RHIBuffer* staging = new RHIBuffer(stagingDesc);
			staging->InitRHI();

			memcpy(staging->GetMappedData(), dt.Vertices.data(), vertexBufferSize);
			memcpy((uint8*)staging->GetMappedData() + vertexBufferSize, dt.Indices.data(), indexBufferSize);

			Graphics::GetRHI().ImmediateSubmit([&](RHICommandBuffer* cmd) {

				Graphics::GetRHI().CopyBuffer(cmd, staging, vb, 0, 0, vertexBufferSize);
				Graphics::GetRHI().CopyBuffer(cmd, staging, ib, vertexBufferSize, 0, indexBufferSize);
				});

			staging->ReleaseRHI();
			delete staging;
			});
	}

	Mesh::~Mesh() {
		SafeResourceRelease(m_VertexBuffer);
		SafeResourceRelease(m_IndexBuffer);
	}

	TRef<Mesh> Mesh::Create(BinaryReadStream& stream, UUID id) {
		char magic[4] = {};
		stream >> magic;

		if (memcmp(MESH_MAGIC, magic, sizeof(char) * 4) != 0) {
			ENGINE_ERROR("Mesh asset file is not valid: {}!", (uint64)id);
			return nullptr;
		}

		AABBBounds bounds{};
		MeshData data{};

		stream >> bounds;
		stream >> data.Vertices >> data.Indices >> data.SubMeshes;

		return CreateRef<Mesh>(std::move(data), &bounds, id);
	}

    TRef<Mesh> Mesh::Create(MeshData&& data, AABBBounds* bounds) {
		return CreateRef<Mesh>(std::move(data), bounds, 0);
	}
}