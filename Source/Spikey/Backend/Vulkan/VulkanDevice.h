#pragma once

#include <Backend/Vulkan/VulkanCommon.h>
#include <Engine/Graphics/GraphicsCore.h>
#include <Engine/Core/Window.h>

namespace Spikey {

	class VulkanCommandList;

	class VulkanQueue {
	public:
		VulkanQueue(EQueueType type, VkQueue queue, uint32 familyIndex);
		~VulkanQueue();

		VulkanCommandList* CreateCommandList();

		void AddWaitSemaphore(VkSemaphore wait, uint64 value = 1);
		void AddSignalSemaphore(VkSemaphore signal, uint64 value = 1);

		void Submit(VulkanCommandList* cmd);
		void WaitCommandList(VulkanCommandList* cmd, uint64 timeout);

		EQueueType GetType() const { return m_Type; }
		uint32 GetFamilyIndex() const { return m_FamilyIndex; }

	private:
		class VulkanDevice* m_Device;

		VkSemaphore m_TrackingSemaphore;
		VkQueue m_Queue;
		EQueueType m_Type;
		uint32 m_FamilyIndex;

		std::mutex m_Mutex;
		std::vector<std::pair<VkSemaphore, uint64>> m_WaitSemaphores;
		std::vector<std::pair<VkSemaphore, uint64>> m_SignalSemaphores;

		uint64 m_SubmitCounter;
		uint64 m_LastFinishedID;

		std::vector<VulkanCommandList*> m_ListsInFlight;
		std::vector<VulkanCommandList*> m_ListPool;
	};

	class VulkanDevice {
	public:
		VulkanDevice(IWindow* window, bool useValidationLayers = false);
		~VulkanDevice();

		VkInstance GetVkInstance() const { return m_Instance; }
		VkDevice GetVkDevice() const { return m_Device; }
		VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
		VmaAllocator GetAllocator() const { return m_Allocator; }
		const VkPhysicalDeviceLimits& GetLimits() const { return m_Limits; }

	private:
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice;
		VkPhysicalDeviceLimits m_Limits;
		VkDevice m_Device;
		VkSurfaceKHR m_Surface;

		VmaAllocator m_Allocator;
	};
}