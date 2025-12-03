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
		VkCommandPool    m_Pool;
		VkCommandBuffer  m_CmdBuffer;
		uint64           m_SubmitID;
	};

	class VulkanBuffer : public IRHIBuffer {
	public:
		VulkanBuffer(uint64 size, EBufferUsage usage, VulkanRHIDevice& device);
		virtual ~VulkanBuffer() override;

		virtual void* GetMappedData() const override { return m_MappedData; }
		virtual void* GetNative() const override { return (void*)m_Buffer; }
		VkBuffer      GetBufferHandle() const { return m_Buffer; }

	private:
		VulkanRHIDevice& m_Device;
		VkBuffer         m_Buffer;
		VmaAllocation    m_Allocation;
		void*            m_MappedData;
	};

	class VulkanTexture2D : public IRHITexture2D {
	public:
		VulkanTexture2D(uint32 width, uint32 height, uint32 numMips, ETextureFormat format, ETextureUsage usage, VulkanRHIDevice& device);
		virtual ~VulkanTexture2D() override;

		VkImage       GetImageHandle() const { return m_Image; }
		virtual void* GetNative() const override { return (void*)m_Image; }

	private:
		VulkanRHIDevice& m_Device;
		VkImage          m_Image;
		VmaAllocation    m_Allocation;
	};

	class VulkanTextureCube : public IRHITextureCube {
	public:
		VulkanTextureCube(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage, VulkanRHIDevice& device);
		virtual ~VulkanTextureCube() override;

		VkImage       GetImageHandle() const { return m_Image; }
		virtual void* GetNative() const override { return (void*)m_Image; }

	private:
		VulkanRHIDevice& m_Device;
		VkImage          m_Image;
		VmaAllocation    m_Allocation;
	};

	class VulkanTextureView : public IRHITextureView {
	public:
		VulkanTextureView(const SubresourceRange& range, IRHITexture* tex, VulkanRHIDevice& device);
		virtual ~VulkanTextureView() override;

		VkImageView   GetViewHandle() const { return m_View; }
		virtual void* GetNative() const override { return (void*)m_View; }
	private:
		VulkanRHIDevice& m_Device;
		VkImageView      m_View;
	};

	class VulkanSamplerState : public IRHISamplerState {
	public:
		VulkanSamplerState(const SamplerStateDesc& desc, VulkanRHIDevice& device);
		virtual ~VulkanSamplerState() override;

		VkSampler     GetSamplerHandle() const { return m_Sampler; }
		virtual void* GetNative() const override { return (void*)m_Sampler; }

	private:
		VulkanRHIDevice& m_Device;
		VkSampler        m_Sampler;
	};

	// to remove overlapping descriptors
	constexpr uint32 VK_BINDING_SHIFT_B = 0;
	constexpr uint32 VK_BINDING_SHIFT_T = 1000;
	constexpr uint32 VK_BINDING_SHIFT_U = 2000;
	constexpr uint32 VK_BINDING_SHIFT_S = 3000;
	constexpr uint32 VK_IMMUTABLE_SAMPLER_FIRST_SLOT = 100;

	class VulkanPipelineState : public IRHIPipelineState {
	public:
		VulkanPipelineState(const PipelineStateDesc& desc, VulkanRHIDevice& device);
		virtual ~VulkanPipelineState() override;

		VkPipelineLayout      GetLayoutHandle() const { return m_Layout; }
		VkPipeline            GetPipelineHandle() const { return m_Pipeline; }
		VkDescriptorSetLayout GetSetLayoutHandle() const { return m_SetLayout; }
		VkPushConstantRange   GetPushConstants() const { return m_PushConstants; }

		const std::vector<VkDescriptorSetLayoutBinding>& GetLayoutBindings() const {
			return m_LayoutBindings; 
		}

	private:
		VulkanRHIDevice&                          m_Device;
		VkPipelineLayout                          m_Layout;
		VkPipeline                                m_Pipeline;
		VkDescriptorSetLayout                     m_SetLayout;
		VkPushConstantRange                       m_PushConstants;
		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;
	};

	class VulkanShader : public IRHIShader {
	public:
		VulkanShader(const std::span<uint8>& bytecode, VkShaderStageFlags stage, VulkanRHIDevice& device);
		virtual ~VulkanShader() override;

		VkPushConstantRange GetPushConstants() const { return m_PushConstants; }
		VkShaderModule      GetShaderHandle() const { return m_Module; }
		virtual void*       GetNative() const override { return (void*)m_Module; }

		using BindingsArray = std::vector<VkDescriptorSetLayoutBinding>;
		const BindingsArray& GetBindings() const { return m_Bindings; }

	private:
		VulkanRHIDevice&    m_Device;
		VkShaderModule      m_Module;
		
		VkPushConstantRange m_PushConstants;
		BindingsArray       m_Bindings;
	};

	struct VulkanPSOLayout {
		VkPipelineLayout      Layout;
		VkDescriptorSetLayout SetLayout;
	};

	struct VulkanPSOLayoutHash {
		std::vector<VkDescriptorSetLayoutBinding> Bindings;
		VkPushConstantRange                       PushConstants;
		uint64                                    Hash;

		bool operator==(const VulkanPSOLayoutHash& other) const {
			if (Hash != other.Hash)
				return false;
			if (Bindings.size() != other.Bindings.size())
				return false;
			if (
				PushConstants.offset != other.PushConstants.offset ||
				PushConstants.size != other.PushConstants.size     ||
				PushConstants.stageFlags != other.PushConstants.stageFlags
				)
				return false;

			for (int i = 0; i < Bindings.size(); i++) {
				const auto& a = Bindings[i];
				const auto& b = other.Bindings[i];

				if (
					a.binding != b.binding                 ||
					a.descriptorCount != b.descriptorCount ||
					a.descriptorType != b.descriptorType   ||
					a.stageFlags != b.stageFlags
					)
					return false;
			}

			return true;
		}

		void ComputeHash() {
			Hash = 0;

			for (auto& x : Bindings) {
				Math::HashCombine(Hash, x.binding);
				Math::HashCombine(Hash, x.descriptorCount);
				Math::HashCombine(Hash, x.descriptorType);
				Math::HashCombine(Hash, x.stageFlags);
			}

			Math::HashCombine(Hash, PushConstants.offset);
			Math::HashCombine(Hash, PushConstants.size);
			Math::HashCombine(Hash, PushConstants.stageFlags);
		}
	};
}

namespace std {
	template<>
	struct hash<Spikey::VulkanPSOLayoutHash> {
		constexpr size_t operator()(const Spikey::VulkanPSOLayoutHash& hash) const {
			return hash.Hash;
		}
	};
}