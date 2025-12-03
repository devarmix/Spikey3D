#pragma once

#include <Engine/Graphics/GraphicsCore.h>
#include <Backend/Vulkan/VulkanResource.h>

namespace Spikey {

	class VulkanQueue {
	public:
		VulkanQueue(EQueueType type, VkQueue queue, uint32 familyIndex);
		~VulkanQueue();

		VulkanCommandList* CreateCommandList();

		void AddWaitSemaphore(VkSemaphore wait, uint64 value = 1);
		void AddSignalSemaphore(VkSemaphore signal, uint64 value = 1);

		void Submit(VulkanCommandList* cmd);
		void WaitCommandList(VulkanCommandList* cmd, uint64 timeout);

		ERHIQueue GetType() const { return m_Type; }
		uint32 GetFamilyIndex() const { return m_FamilyIndex; }

	private:
		class VulkanRHIDevice& m_Device;

		VkSemaphore m_TrackingSemaphore;
		VkQueue m_Queue;
		ERHIQueue m_Type;
		uint32 m_FamilyIndex;

		std::mutex m_Mutex;
		std::vector<std::pair<VkSemaphore, uint64>> m_WaitSemaphores;
		std::vector<std::pair<VkSemaphore, uint64>> m_SignalSemaphores;

		uint64 m_SubmitCounter;
		uint64 m_LastFinishedID;

		std::vector<VulkanCommandList*> m_ListsInFlight;
		std::vector<VulkanCommandList*> m_ListPool;
	};

	class VulkanRHIDevice : public IRHIDevice {
	public:
		VulkanRHIDevice(IWindow* window, bool validationLayers = true);
		virtual ~VulkanRHIDevice() override;

		VkInstance       GetInstanceHandle() const { return m_Instance; }
		VkDevice         GetDeviceHandle() const { return m_Device; }
		VmaAllocator     GetAllocatorHandle() const { return m_Allocator; }
		VulkanQueue&     GetQueue(ERHIQueue queue) const { return *m_Queues[(size_t)queue]; }
		VulkanPSOLayout  CreateCachedPSOLayout(const VulkanPSOLayoutHash& hash);
		VkPipelineCache  GetPipelineCacheHandle() const { return m_PipelineStateCache; }

		const VkSampler*                  GetImmutableSamplers() const { return m_ImmutableSamplers.data(); }
		const VkPhysicalDeviceLimits&     GetLimits() const { return m_Limits; }
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
}