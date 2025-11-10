#pragma once
#include <Engine/Graphics/Resource.h>

namespace Spikey {

	enum class EBufferMemUsage : uint8 {
		None = 0,

		CPUToGPU,
	    CPUOnly,
		GPUOnly
	};

	enum class EBufferUsage : uint8 {
		None = 0,

		Constant     = BIT(0),
		Storage      = BIT(1),
		CopySrc      = BIT(2),
		CopyDst      = BIT(3),
		Indirect     = BIT(4),
		Addressable  = BIT(5),
		Index        = BIT(6)
	};
	ENUM_FLAGS_OPERATORS(EBufferUsage);

	struct BufferDesc {

		uint64 Size;
		EBufferUsage UsageFlags;
		EBufferMemUsage MemUsage;

		bool operator==(const BufferDesc& other) const {

			return (Size == other.Size
				&& MemUsage == other.MemUsage
				&& UsageFlags == other.UsageFlags);
		}
	};

	class RHICommandBuffer;

	class RHIBuffer : public IRHIResource {
	public:
		RHIBuffer(const BufferDesc& desc) 
			: m_Desc(desc), m_RHIData(0), m_MappedData(nullptr), m_GPUAddress(0), m_LastAccess(EGPUAccess::None) 
		{
		}

		virtual ~RHIBuffer() override {}

		virtual void InitRHI() override;
		virtual void ReleaseRHI() override;
		virtual void ReleaseRHIImmediate() override;

		uint64 GetSize() const { return m_Desc.Size; }
		EBufferUsage GetUsage() const { return m_Desc.UsageFlags; }
		EBufferMemUsage GetMemUsage() const { return m_Desc.MemUsage; }

		RHIData GetRHIData() const { return m_RHIData; }
		void* GetMappedData() { return m_MappedData; }

		const BufferDesc& GetDesc() { return m_Desc; }
		uint64_t GetGPUAddress() const { return m_GPUAddress; }

		void Barrier(RHICommandBuffer* cmd, uint64 size, uint64 offset, EGPUAccess newAccess);
		void Barrier(RHICommandBuffer* cmd, EGPUAccess newAccess);

	private:
		void* m_MappedData;
		RHIData m_RHIData;

		BufferDesc m_Desc;
		EGPUAccess m_LastAccess;
		uint64 m_GPUAddress;
	};
}