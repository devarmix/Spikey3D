#include <Engine/Graphics/Buffer.h>
#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Graphics/FrameRenderer.h>

namespace Spikey {

	void RHIBuffer::InitRHI() {
		m_RHIData = Graphics::GetRHI().CreateBufferRHI(m_Desc);

		if (m_Desc.MemUsage == EBufferMemUsage::CPUOnly || m_Desc.MemUsage == EBufferMemUsage::CPUToGPU)
			m_MappedData = Graphics::GetRHI().MapBufferMem(this);

		if (EnumHasAllFlags(m_Desc.UsageFlags, EBufferUsage::Addressable)) {
			m_GPUAddress = Graphics::GetRHI().GetBufferGPUAddress(this);
		}
	}

	void RHIBuffer::ReleaseRHIImmediate() {
		Graphics::GetRHI().DestroyBufferRHI(m_RHIData);
	}

	void RHIBuffer::ReleaseRHI() {
		Graphics::GetFrameRenderer().EnqueueDeferred([data = m_RHIData]() {
			Graphics::GetRHI().DestroyBufferRHI(data);
			});
	}

	void RHIBuffer::Barrier(RHICommandBuffer* cmd, uint64 size, uint64 offset, EGPUAccess newAccess) {
		Graphics::GetRHI().BarrierBuffer(cmd, this, size, offset, m_LastAccess, newAccess);
		m_LastAccess = newAccess;
	}

	void RHIBuffer::Barrier(RHICommandBuffer* cmd, EGPUAccess newAccess) {
		Barrier(cmd, m_Desc.Size, 0, newAccess);
	}
}