#pragma once
#include <Engine/Core/RefCounted.h>

namespace Spikey {

	enum class EBufferUsage : uint8 {
		None = 0,

		Constant     = BIT(0),
		Storage      = BIT(1),
		CopySrc      = BIT(2),
		CopyDst      = BIT(3),
		Indirect     = BIT(4),
		Index        = BIT(5),
		Mapped       = BIT(6)
	};
	ENUM_FLAGS_OPERATORS(EBufferUsage);

	class IRHIBuffer : public IRefCounted {
	public:
		IRHIBuffer(uint64 size, EBufferUsage usage)
			: m_Size(size), m_UsageFlags(usage), m_LastAccess(EGPUAccess::None)
		{
		}

		uint64        GetSize() const { return m_Size; }
		EBufferUsage  GetUsage() const { return m_UsageFlags; }
		virtual void* GetMappedData() const = 0;
		virtual void* GetNative() const = 0;

		void Barrier(IRHICommandList* cmd, uint64 size, uint64 offset, EGPUAccess newAccess) {
			cmd->BarrierBuffer(this, size, offset, m_LastAccess, newAccess);
			m_LastAccess = newAccess;
		}

		void Barrier(class IRHICommandList* cmd, EGPUAccess newAccess) { Barrier(cmd, m_Size, 0, newAccess); }

	private:
		uint64 m_Size;
		EBufferUsage m_UsageFlags;
		EGPUAccess m_LastAccess;
	};

	using BufferRHIRef = TRef<IRHIBuffer>;
}