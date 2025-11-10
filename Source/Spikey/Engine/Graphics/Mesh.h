#pragma once
#include <Engine/Graphics/Material.h>

namespace Spikey {

	struct alignas(16) Vertex {

		Vec3 Position;
		uint32 Color;

		Vec2 UV0;
		Vec2 UV1;

		PackedHalf Tangent;
		PackedHalf Normal;
	};

	struct SubMesh {

		uint32 FirstIndex;
		uint32 IndexCount;
	};

	constexpr char MESH_MAGIC[4] = { 'S', 'M', 'S', 'F' };

	struct MeshDesc {

		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
		std::vector<SubMesh> SubMeshes;

		bool NeedCPUData = false;
	};

	struct AABBBounds {
		Vec3 LowerBound;
		Vec3 UpperBound;
	};

	class RHIMesh : public IRHIResource {
	public:
		RHIMesh(const MeshDesc& desc);
		virtual ~RHIMesh() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;

		RHIBuffer* GetVertexBuffer() { return m_VertexBuffer; }
		RHIBuffer* GetIndexBuffer() { return m_IndexBuffer; }

		const std::vector<Vertex>& GetVertices() const { return m_Desc.Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Desc.Indices; }

		const AABBBounds& GetBounds() const { return m_Bounds; }
		const MeshDesc& GetDesc() const { return m_Desc; }

	private:
		MeshDesc m_Desc;

		RHIBuffer* m_VertexBuffer;
		RHIBuffer* m_IndexBuffer;

		AABBBounds m_Bounds;
	};

	class Mesh : public IAsset {
	public:
		Mesh(const MeshDesc& desc, UUID id);
		virtual ~Mesh() override;

		static TRef<Mesh> Create(BinaryReadStream& stream, UUID id);

		RHIMesh* GetResource() { return m_RHIResource; }
		void ReleaseResource();
		void CreateResource(const MeshDesc& desc);

		const std::vector<Vertex>& GetVertices() const { return m_RHIResource->GetVertices(); }
		const std::vector<uint32_t>& GetIndices() const { return m_RHIResource->GetIndices(); }

	private:
		RHIMesh* m_RHIResource;
	};
}