#pragma once

#include <Core/Common.h>
#include <Core/RefCounted.h>

namespace Spikey 
{
	enum class BufferFlags : uint8
	{
		None = 0,
		ShaderResource = BIT(0),
		Storage        = BIT(1),
		Indirect       = BIT(2),
		IndexBuffer    = BIT(3),
		VertexBuffer   = BIT(4),
	    Upload         = BIT(5),
		ReadBack       = BIT(6),
		GPUAddress     = BIT(7)
	};
	ENUM_FLAGS_OPERATORS(BufferFlags);

	struct BufferRange
	{
		uint64 ByteSize;
		uint64 ByteOffset;
	};

	struct BufferDesc
	{
		uint64      ByteSize;
		uint32      Stride; // size of a single element
		BufferFlags Flags;
	};

	class GPUBuffer : public RefCounted
	{
	protected:
		GPUBuffer(const BufferDesc& desc)
			: m_Desc(desc)
		{
		}

		BufferDesc m_Desc;

	public:
		uint64 GetSize() const { return m_Desc.ByteSize; }
		uint32 GetStride() const { return m_Desc.Stride; }
		BufferFlags GetFlags() const { return m_Desc.Flags; }

		const BufferDesc& GetDesc()
		{
			return m_Desc;
		}

		virtual void* GetMappedData() const = 0;
		virtual void* GetNative() const = 0;
	};
}