#pragma once

#include <Backend/Vulkan/VulkanCommon.h>
#include <Engine/Graphics/GraphicsCore.h>

namespace Spikey {

	class VulkanCommandList;
	class VulkanDevice;

	class VulkanQueue
	{
	public:
		VkSemaphore m_TrackingSemaphore;

		VulkanQueue(ERHIQueue queueID, VkQueue queue, uint32 familyIndex, VulkanDevice& device);
		~VulkanQueue();

		TRefCountPtr<VulkanCommandList> CreateCommandList();

		void AddWaitSemaphore(VkSemaphore wait, uint64 value);
		void AddSignalSemaphore(VkSemaphore signal, uint64 value);

		void Submit(const VulkanCommandList** cmds, uint64 numCmd);
		bool WaitCommandList(VulkanCommandList* cmd, uint64 timeout);
		void RetireCommandLists();

		ERHIQueue GetQueueID() const { return m_QueueID; }
		uint32    GetFamilyIndex() const { return m_FamilyIndex; }

	private:
		uint64 UpdateLastFinishedID();
		bool   PollCommandList(uint64 cmdID);

	private:
		VulkanDevice& m_Device;
		VkQueue       m_Queue;
		ERHIQueue     m_QueueID;
		uint32        m_FamilyIndex;

		std::vector<VkSemaphore> m_WaitSemaphores;
		std::vector<uint64>      m_WaitSemaphoreValues;
		std::vector<VkSemaphore> m_SignalSemaphores;
		std::vector<uint64>      m_SignalSemaphoreValues;

		uint64 m_LastSubmitID = 0;
		uint64 m_LastFinishedID = 0;

		std::mutex                                   m_Mutex;
		std::vector<TRefCountPtr<VulkanCommandList>> m_ListsInFlight;
		std::vector<TRefCountPtr<VulkanCommandList>> m_ListsPool;
	};

	class VulkanDevice : public RHIDevice {
	public:
		VulkanRHIDevice(IWindow* window, bool validationLayers = true);
		virtual ~VulkanRHIDevice() override;

		VkInstance       GetInstanceHandle() const { return m_Instance; }
		VkDevice         GetDeviceHandle() const { return m_Device; }
		VmaAllocator     GetAllocatorHandle() const { return m_Allocator; }
		VulkanQueue&     GetQueue(ERHIQueue queue) const { return *m_Queues[(size_t)queue]; }
		VulkanPSOLayout  CreateCachedPSOLayout(const VulkanPSOLayoutHash& hash);
		VkPipelineCache  GetPipelineCacheHandle() const { return m_PipelineStateCache; }

		const VkSampler* GetImmutableSamplers() const { return m_ImmutableSamplers.data(); }
		const VkPhysicalDeviceLimits& GetLimits() const { return m_Limits; }
		const VkPhysicalDeviceProperties& GetProperties() const { return m_Properties; }

		struct ResourceDestroyer {
			VkImage        Image = nullptr;
			VkImageView    View = nullptr;
			VkBuffer       Buffer = nullptr;
			VmaAllocation  Allocation = nullptr;
		};

		void DestroyResource(const ResourceDestroyer& destroyer);

	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice;
		VkPhysicalDeviceLimits m_Limits;
		VkPhysicalDeviceProperties m_Properties;
		VkDevice m_Device;
		VkSurfaceKHR m_Surface;
		VmaAllocator m_Allocator;
		VulkanQueue* m_Queues[(size_t)ERHIQueue::COUNT];

		VkPipelineCache m_PipelineStateCache;

		std::mutex m_DestructionMutex;
		std::deque<std::pair<ResourceDestroyer, uint64>> m_DestructionQueue;

		std::unordered_map<VulkanPSOLayoutHash, VkPipelineLayout> m_PSOLayoutCache;

		std::mutex m_SamplerCacheLock;
		std::unordered_map<SamplerStateDesc, SamplerStateRHIRef> m_SamplerCache;
		std::vector<VkSampler> m_ImmutableSamplers;
	};

	class VulkanCommandList : public RHICommandList
	{
	public:
		uint64 SubmissionID;

		VulkanCommandList(VulkanQueue* queue, VulkanDevice& device);
		virtual ~VulkanCommandList() override;

		virtual void CopyTexture(RHITexture* src, const TextureSlice& srcSlice, RHITexture* dst, const TextureSlice& dstSlice) override;
		virtual void FlushBarriers() override;

		VkCommandBuffer GetCmdHandle() const { return m_CmdBuffer; }
		virtual void*   GetNative() const override { return (void*)m_CmdBuffer; }

	private:
		VulkanDevice&   m_Device;
		VkCommandPool   m_Pool;
		VkCommandBuffer m_CmdBuffer;
	};

	class VulkanBuffer : public RHIBuffer 
	{
	public:
		VulkanBuffer(uint64 size, EBufferFlags flags, VulkanRHIDevice& device);
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

	class VulkanTexture : public RHITexture 
	{
	public:
		VulkanTexture(const TextureDesc& desc, VulkanRHIDevice& device);
		virtual ~VulkanTexture() override;

		enum class ESubresourceViewType : uint8
		{
			AllAspects,
			DepthOnly,
			StencilOnly
		};

		struct SubresourceViewKey 
		{
			TextureSubresourceSet Subresources;
			ETextureDimension     Dimension;
			ETextureFormat        Format;
			ESubresourceViewType  ViewType;
		};

		VkImageView GetSubresourceView(const TextureSubresourceSet& subresource, ETextureDimension dimension,
			ETextureFormat format, ESubresourceViewType viewType);
		VkImage       GetImageHandle() const { return m_Image; }
		virtual void* GetNative() const override { return (void*)m_Image; }

	private:
		VulkanRHIDevice& m_Device;
		VkImage          m_Image;
		VmaAllocation    m_Allocation;

		std::mutex       m_Mutex;
		std::unordered_map<SubresourceViewKey, VkImageView> m_ViewsMap;
	};

	class VulkanSamplerState : public RHISamplerState 
	{
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

	class VulkanPipelineState : public RHIPipelineState 
	{
	public:
		VulkanPipelineState(const PipelineStateDesc& desc, VulkanRHIDevice& device);
		virtual ~VulkanPipelineState() override;

		VkPipelineLayout      GetLayoutHandle() const { return m_Layout; }
		VkPipeline            GetPipelineHandle() const { return m_Pipeline; }
		VkDescriptorSetLayout GetSetLayoutHandle() const { return m_SetLayout; }
		VkPushConstantRange   GetPushConstants() const { return m_PushConstants; }

		const std::vector<VkDescriptorSetLayoutBinding>& GetLayoutBindings() const 
		{
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