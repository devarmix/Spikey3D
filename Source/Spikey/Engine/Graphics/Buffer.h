#pragma once
#include <Engine/Graphics/RHIResource.h>

namespace Spikey {

	enum class EBufferFlags : uint8
	{
		None = 0,
		Constant     = BIT(0),
		Storage      = BIT(1),
		Indirect     = BIT(2),
		Index        = BIT(3),
	    Upload       = BIT(4),
		ReadBack     = BIT(5),
		GPUAddress   = BIT(6)
	};
	ENUM_FLAGS_OPERATORS(EBufferFlags);

	class RHIBuffer : public IRHIResource
	{
	public:
		RHIBuffer(uint64 size, EBufferFlags flags)
			: m_Size(size), m_Flags(flags)
		{
		}

		uint64        GetSize() const { return m_Size; }
		EBufferFlags  GetUsage() const { return m_Flags; }
		virtual void* GetMappedData() const = 0;

	private:
		uint64       m_Size;
		EBufferFlags m_Flags;
	};

	using BufferRHIRef = TRef<RHIBuffer>;
}