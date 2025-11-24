#pragma once

#include <Backend/Vulkan/VulkanCommon.h>
#include <Engine/Graphics/GraphicsCore.h>

namespace Spikey {

	class VulkanRHIDevice;
	class VulkanQueue;

	class VulkanCommandList : public IRHICommandList {
	public:
		VulkanCommandList(ERHIQueue queue, VulkanRHIDevice& device);
		virtual ~VulkanCommandList() override;

		virtual void MipMapTexture2D(IRHITexture2D* tex, EGPUAccess lastAccess, EGPUAccess newAccess, uint32 numMips) override;
		virtual void CopyTexture(IRHITexture* src, const TextureCopyRegion& srcRegion, IRHITexture* dst, const TextureCopyRegion& dstRegion, Vec2Uint copySize) override;
		virtual void ClearTexture(IRHITexture* tex, const SubresourceRange& range, EGPUAccess access, const Vec4& color) override;
		virtual void CopyFromTextureToCPU(IRHITexture* src, const SubresourceCopyRegion& region, IRHIBuffer* dst) override;
		virtual void BarrierTexture(IRHITexture* texture, const TextureBarrierRegion* regions, uint32 numRegions) override;
		virtual void CopyBuffer(IRHIBuffer* srcBuffer, IRHIBuffer* dstBuffer, uint64 srcOffset, uint64 dstOffset, uint64 size) override;
		virtual void BarrierBuffer(IRHIBuffer* buffer, uint64 size, uint64 offset, EGPUAccess lastAccess, EGPUAccess newAccess) override;
		virtual void FillBuffer(IRHIBuffer* buffer, uint64 size, uint64 offset, uint32 value) override;

		void SetSubmited(uint64 id) {
			m_SubmitID = id;
			vkEndCommandBuffer(m_CmdBuffer);
		}

		void Reset() {
			m_SubmitID = 0;
			vkResetCommandBuffer(m_CmdBuffer, 0);
		}

		VkCommandBuffer GetBufferHandle() const { return m_CmdBuffer; }
		virtual void* GetNative() const override { return (void*)m_CmdBuffer; }

	private:
		VulkanRHIDevice& m_Device;
		VkCommandPool m_Pool;
		VkCommandBuffer m_CmdBuffer;
		uint64 m_SubmitID;
	};

	class VulkanBuffer : public IRHIBuffer {
	public:
		VulkanBuffer(uint64 size, EBufferUsage usage, VulkanRHIDevice& device);
		virtual ~VulkanBuffer() override;

		virtual int32 GetSRVHandle() override;
		virtual int32 GetUAVHandle() override;
		virtual void* GetMappedData() const override { return m_MappedData; }

		VkBuffer GetBufferHandle() const { return m_Buffer; }
		virtual void* GetNative() const override { return (void*)m_Buffer; }

	private:
		VulkanRHIDevice& m_Device;
		VkBuffer m_Buffer;
		VmaAllocation m_Allocation;
		int32 m_SRVHandle;
		int32 m_UAVHandle;
		void* m_MappedData;
	};

	class VulkanTexture2D : public IRHITexture2D {
	public:
		VulkanTexture2D(uint32 width, uint32 height, uint32 numMips, ETextureFormat format, ETextureUsage usage, VulkanRHIDevice& device);
		virtual ~VulkanTexture2D() override;

		VkImage GetImageHandle() const { return m_Image; }
		virtual void* GetNative() const override { return (void*)m_Image; }

	private:
		VulkanRHIDevice& m_Device;
		VkImage m_Image;
		VmaAllocation m_Allocation;
	};

	class VulkanTextureCube : public IRHITextureCube {
	public:
		VulkanTextureCube(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage, VulkanRHIDevice& device);
		virtual ~VulkanTextureCube() override;

		VkImage GetImageHandle() const { return m_Image; }
		virtual void* GetNative() const override { return (void*)m_Image; }

	private:
		VulkanRHIDevice& m_Device;
		VkImage m_Image;
		VmaAllocation m_Allocation;
	};

	class VulkanTextureView : public IRHITextureView {
	public:
		VulkanTextureView(const SubresourceRange& range, IRHITexture* tex, VulkanRHIDevice& device);
		virtual ~VulkanTextureView() override;

		virtual int32 GetSRVHandle() override;
		virtual int32 GetUAVHandle() override;

		VkImageView GetViewHandle() const { return m_View; }
		virtual void* GetNative() const override { return (void*)m_View; }
	private:
		VulkanRHIDevice& m_Device;
		VkImageView m_View;
		int32 m_SRVHandle;
		int32 m_UAVHandle;
	};

	class VulkanSamplerState : public IRHISamplerState {
	public:
	private:
	};

	class VulkanShader : public IRHIShader {
	public:
		VulkanShader(const std::span<uint8>& bytecode, VulkanRHIDevice& device);
		virtual ~VulkanShader() override;

		VkShaderModule GetShaderHandle() const { return m_Module; }
		virtual void* GetNative() const override { return (void*)m_Module; }

	private:
		VulkanRHIDevice& m_Device;
		VkShaderModule m_Module;
	};

	class VulkanPipelineState : public IRHIPipelineState {
	public:
		VulkanPipelineState(const PipelineStateDesc& desc, VulkanRHIDevice& device);
		virtual ~VulkanPipelineState() override;

		VkPipelineLayout GetLayoutHandle() const { return m_Layout; }
		VkPipeline GetPipelineHandle() const { return m_Pipeline; }

	private:
		VulkanRHIDevice& m_Device;
		VkPipelineLayout m_Layout;
		VkPipeline m_Pipeline;
	};
}