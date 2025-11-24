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

	struct AABBBounds {
		Vec3 LowerBound;
		Vec3 UpperBound;
	};

	struct MeshData {
		std::vector<Vertex> Vertices;
		std::vector<uint32> Indices;
		std::vector<SubMesh> SubMeshes;
	};

	void CalculateMeshBounds(const std::vector<Vertex>& vertices, AABBBounds& out);

	class Mesh : public IAsset {
	public:
		Mesh(MeshData&& data, AABBBounds* bounds, UUID id);
		virtual ~Mesh() override;

		static TRef<Mesh> Create(BinaryReadStream& stream, UUID id);
		static TRef<Mesh> Create(MeshData&& data, AABBBounds* bounds = nullptr);

		RHIBufferRef GetVertexBuffer() { return m_VertexBuffer; }
		RHIBufferRef GetIndexBuffer() { return m_IndexBuffer; }
		const AABBBounds& GetBounds() const { return m_Bounds; }

		uint32 GetNumSubMeshes() const { return (uint32)m_SubMeshes.size(); }
		const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }

	private:
		RHIBufferRef m_VertexBuffer;
		RHIBufferRef m_IndexBuffer;

		std::vector<SubMesh> m_SubMeshes;
		AABBBounds m_Bounds;
	};
}