#include <Vulkan/VulkanBackend.h>
#include <Vulkan/VulkanTools.h>
#include <Core/Engine.h>

#include <spirv_reflect.h>
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>

#define USE_VULKAN_PIPELINE_CACHE 0

namespace Spikey::Vulkan
{
	VulkanDevice::VulkanDevice() 
		: m_PipelineStateCache(VK_NULL_HANDLE)
	{
		vkb::InstanceBuilder builder{};
		auto instance = builder
			.set_app_name(Application::GetName())
#ifdef _DEBUG
			.request_validation_layers(true)
#endif
			.use_default_debug_messenger()
			.require_api_version(1, 4, 0)
			.build();

		vkb::Instance vkbInstance = instance.value();

		m_Instance = vkbInstance.instance;
		m_DebugMessenger = vkbInstance.debug_messenger;

		VkPhysicalDeviceVulkan13Features features13{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		features13.dynamicRendering = true;
		features13.synchronization2 = true;

		// VkPhysicalDeviceVulkan12Features features12{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		// features12.bufferDeviceAddress = true;

		vkb::PhysicalDeviceSelector selector{ vkbInstance };
		vkb::PhysicalDevice physicalDevice = selector
			.set_minimum_version(1, 4)
			.set_required_features_13(features13)
			// .set_required_features_12(features12)
			.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
			.select()
			.value();

		/*
		std::vector<vkb::CustomQueueDescription> queueDescriptions{};
		{
			auto queueFamilyProperties = physicalDevice.get_queue_families();
			int32 graphicsFamily = -1;
			int32 computeFamily = -1;
			int32 transferFamily = -1;

			for (int32 i = 0; i < queueFamilyProperties.size(); i++)
			{
				const VkQueueFamilyProperties& props = queueFamilyProperties[i];

				if (props.queueCount > 0)
				{
					bool isQueueUsed = false;

					if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT
						&& graphicsFamily == -1)
					{
						graphicsFamily = i;
						isQueueUsed = true;
					}

					if (props.queueFlags & VK_QUEUE_COMPUTE_BIT
						&& !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
						&& computeFamily == -1)
					{
						computeFamily = i;
						isQueueUsed = true;
					}

					if (props.queueFlags & VK_QUEUE_TRANSFER_BIT
						&& !(props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
						&& !(props.queueFlags & VK_QUEUE_COMPUTE_BIT)
						&& transferFamily == -1)
					{
						transferFamily = i;
						isQueueUsed = true;
					}

					if (isQueueUsed)
					{
						auto& queueDesc = queueDescriptions.emplace_back();
						queueDesc.index = i;
						queueDesc.priorities = { 1.0f };
					}
				}
			}

			m_GraphicsQueue = new VulkanQueue(graphicsFamily, this);

			if (computeFamily == -1)
				m_ComputeQueue = m_GraphicsQueue;
			else
				m_ComputeQueue = new VulkanQueue(computeFamily, this);

			if (transferFamily == -1)
				m_TransferQueue = m_ComputeQueue;
			else
				m_TransferQueue = new VulkanQueue(transferFamily, this);
		}
		*/

		{
			vkb::DeviceBuilder deviceBuilder{ physicalDevice };
			//deviceBuilder.custom_queue_setup(queueDescriptions);

			vkb::Device vkbDevice = deviceBuilder.build().value();

			m_Device = vkbDevice.device;
			m_PhysicalDevice = vkbDevice.physical_device;

			m_GraphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
			m_GraphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
		}
		{
			VkFenceCreateInfo fenceInfo
			{
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, 
				.flags = VK_FENCE_CREATE_SIGNALED_BIT 
			};

			for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
			{
				VK_CHECK(vkCreateFence(m_Device, &fenceInfo, nullptr, &FrameFences[i]));
			}

			MainContext = new VulkanCommandContext(m_GraphicsQueue, m_GraphicsQueueFamily, this);
		}
		{
			VmaAllocatorCreateInfo allocatorInfo{};
			allocatorInfo.physicalDevice = m_PhysicalDevice;
			allocatorInfo.device = m_Device;
			allocatorInfo.instance = m_Instance;
			// allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
			allocatorInfo.flags = 0;
			vmaCreateAllocator(&allocatorInfo, &m_Allocator);
		}
		{
			VkBufferCreateInfo bufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			bufferInfo.size = 16;
			bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			bufferInfo.flags = 0;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			VK_CHECK(vmaCreateBuffer(m_Allocator, &bufferInfo, &allocInfo, &NullBuffer, &NullBufferAllocation, nullptr));
		}
		{
			VkImageCreateInfo imageInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
			imageInfo.extent = { 1, 1, 1 };
			imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
			imageInfo.arrayLayers = 1;
			imageInfo.mipLevels = 1;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

			VmaAllocationCreateInfo allocInfo{};
			allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			imageInfo.arrayLayers = 6;
			VK_CHECK(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &NullImage2D, &NullImageAllocation2D, nullptr));

			imageInfo.imageType = VK_IMAGE_TYPE_3D;
			imageInfo.flags = 0;
			imageInfo.arrayLayers = 1;
			VK_CHECK(vmaCreateImage(m_Allocator, &imageInfo, &allocInfo, &NullImage3D, &NullImageAllocation3D, nullptr));

			// transitions for null image descriptors
			{
				VkImageSubresourceRange barrierRange
				{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 6
				};

				MainContext->AddImageBarrier(NullImage2D, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, barrierRange);
				barrierRange.layerCount = 1;
				MainContext->AddImageBarrier(NullImage3D, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, barrierRange);
				MainContext->FlushBarriers();
			}

			VkImageViewCreateInfo viewInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;

			viewInfo.image = NullImage2D;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &NullImageView2D));

			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &NullImageView2DArray));

			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			viewInfo.subresourceRange.layerCount = 6;
			VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &NullImageViewCube));

			viewInfo.image = NullImage3D;
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
			viewInfo.subresourceRange.layerCount = 1;
			VK_CHECK(vkCreateImageView(m_Device, &viewInfo, nullptr, &NullImageView3D));
		}
		{
			VkSamplerCreateInfo samplerInfo{ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			samplerInfo.compareEnable = false;
			samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = FLT_MAX;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.anisotropyEnable = false;
			samplerInfo.maxAnisotropy = 0.0f;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

			// linear clamp
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[0]));

			// linear wrap
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[1]));

			// linear mirror
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[2]));

			// point clamp
			samplerInfo.minFilter = VK_FILTER_NEAREST;
			samplerInfo.magFilter = VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[3]));

			// point wrap
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[4]));

			// point mirror
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[5]));

			// cmp depth
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerInfo.compareEnable = true;
			samplerInfo.compareOp = VK_COMPARE_OP_LESS; // FIXME
			samplerInfo.maxLod = 0.0f;
			VK_CHECK(vkCreateSampler(m_Device, &samplerInfo, nullptr, &StaticSamplers[6]));
		}
		
#if USE_VULKAN_PIPELINE_CACHE
		{
			// TODO: load data from disk
			VkPipelineCacheCreateInfo cacheInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
			cacheInfo.initialDataSize = 0;
			cacheInfo.pInitialData = nullptr;

			VK_CHECK(vkCreatePipelineCache(m_Device, &cacheInfo, nullptr, &m_PipelineStateCache));
		}
#endif
	}

	VulkanDevice::~VulkanDevice()
	{
		WaitGPUIdle();

		// default resources
		{
			vmaDestroyBuffer(m_Allocator, NullBuffer, NullBufferAllocation);
			vmaDestroyImage(m_Allocator, NullImage2D, NullImageAllocation2D);
			vmaDestroyImage(m_Allocator, NullImage3D, NullImageAllocation3D);

			vkDestroyImageView(m_Device, NullImageView2D, nullptr);
			vkDestroyImageView(m_Device, NullImageView2DArray, nullptr);
			vkDestroyImageView(m_Device, NullImageView3D, nullptr);
			vkDestroyImageView(m_Device, NullImageViewCube, nullptr);
		}

		for (auto& [k, v] : m_LayoutCache)
		{
			vkDestroyPipelineLayout(m_Device, v.Layout, nullptr);
			vkDestroyDescriptorSetLayout(m_Device, v.SetLayout, nullptr);
		}

		m_LayoutCache.clear();
		m_SamplerCache.clear();

#if  USE_VULKAN_PIPELINE_CACHE
		vkDestroyPipelineCache(m_Device, m_PipelineStateCache, nullptr);
#endif

		for (uint32 i = 0; i < FRAME_BUFFER_COUNT; i++)
		{
			vkDestroyFence(m_Device, FrameFences[i], nullptr);
		}

		delete MainContext;
		m_DeletionQueue.ReleaseResources(true, this);

		vmaDestroyAllocator(m_Allocator);
		vkDestroyDevice(m_Device, nullptr);
		vkb::destroy_debug_utils_messenger(m_Instance, m_DebugMessenger);
		vkDestroyInstance(m_Instance, nullptr);
	}

	GPUTextureRef VulkanDevice::CreateTexture(const TextureDesc& desc)
	{
		auto texture = TRefCountPtr<VulkanTexture>::Create(desc);
		return GPUTextureRef(texture);
	}

	GPUBufferRef VulkanDevice::CreateBuffer(const BufferDesc& desc)
	{
		auto buffer = TRefCountPtr<VulkanBuffer>::Create(desc);
		return GPUBufferRef(buffer);
	}

	GPUSamplerStateRef VulkanDevice::CreateSamplerState(const SamplerStateDesc& desc)
	{
		std::scoped_lock lock(m_SamplerCacheLock);

		auto it = m_SamplerCache.find(desc);
		if (it != m_SamplerCache.end())
		{
			return it->second;
		}

		GPUSamplerStateRef newSampler = new VulkanSamplerState(desc, this);
		m_SamplerCache.emplace(desc, newSampler);

		return newSampler;
	}

	GPUShaderRef VulkanDevice::CreateShader(std::span<uint8> bytecode, ShaderStage stage)
	{
		auto shader = TRefCountPtr<VulkanShader>::Create(bytecode, stage);
		return GPUShaderRef(shader);
	}

	GPUPipelineStateRef VulkanDevice::CreatePipelineState(const PipelineStateDesc& desc)
	{
		auto pipeline = TRefCountPtr<VulkanPipelineState>::Create(desc);
		return GPUPipelineStateRef(pipeline);
	}

	GPUSwapchain* VulkanDevice::CreateSwapchain(uint32 width, uint32 height, PixelFormat desiredFormat, SDL_Window* window)
	{
		return new VulkanSwapchain(width, height, desiredFormat, window, m_GraphicsQueue, this);
	}

	VulkanPSOLayout VulkanDevice::CreateCachedPSOLayout(const VulkanPSOLayoutHash& hash)
	{
		std::scoped_lock lock(m_LayoutCacheLock);

		auto it = m_LayoutCache.find(hash);
		if (it != m_LayoutCache.end())
		{
			return it->second;
		}

		VulkanPSOLayout newLayout = {};
		VkDescriptorSetLayoutCreateInfo setLayoutInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = (uint32)hash.Bindings.size(),
			.pBindings = hash.Bindings.data()
		};

		VK_CHECK(vkCreateDescriptorSetLayout(m_Device, &setLayoutInfo, nullptr, &newLayout.SetLayout));

		VkPipelineLayoutCreateInfo pipelineInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &newLayout.SetLayout
		};

		if (hash.PushConstants.size > 0)
		{
			pipelineInfo.pushConstantRangeCount = 1;
			pipelineInfo.pPushConstantRanges = &hash.PushConstants;
		}

		VK_CHECK(vkCreatePipelineLayout(m_Device, &pipelineInfo, nullptr, &newLayout.Layout));
		m_LayoutCache.emplace(hash, newLayout);

		return newLayout;
	}

	void VulkanDevice::BeginFrame()
	{
		// timit frames in flight to FRAME_BUFFER_COUNT
		VkFence fence = FrameFences[Engine::FrameCounter % FRAME_BUFFER_COUNT];
		VK_CHECK(vkWaitForFences(m_Device, 1, &fence, true, 1'000'000'000));
		VK_CHECK(vkResetFences(m_Device, 1, &fence));

		MainContext->BeginFrame();
	}

	void VulkanDeletionQueue::ReleaseResources(bool immediate, VulkanDevice* device)
	{
		std::scoped_lock lock(m_Mutex);
		const uint32 framesBeforeDelete = 2;

		while (!m_Entries.empty())
		{
			if ((m_Entries.front().FrameNumber + framesBeforeDelete < Engine::FrameCounter) || immediate)
			{
				auto item = m_Entries.front();
				m_Entries.pop_front();

				switch (item.StructureType)
				{
				case Type::Buffer:
					vkDestroyBuffer(device->GetDeviceHandle(), (VkBuffer)item.Handle, nullptr);
					break;
				case Type::Image:
					vkDestroyImage(device->GetDeviceHandle(), (VkImage)item.Handle, nullptr);
					break;
				case Type::ImageView:
					vkDestroyImageView(device->GetDeviceHandle(), (VkImageView)item.Handle, nullptr);
					break;
				case Type::Pipeline:
					vkDestroyPipeline(device->GetDeviceHandle(), (VkPipeline)item.Handle, nullptr);
					break;
				case Type::Sampler:
					vkDestroySampler(device->GetDeviceHandle(), (VkSampler)item.Handle, nullptr);
					break;
				case Type::DescriptorPool:
					vkDestroyDescriptorPool(device->GetDeviceHandle(), (VkDescriptorPool)item.Handle, nullptr);
					break;
				default:
					assert(0);
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	void VulkanDeletionQueue::EnqueueGeneric(Type type, uint64 handle, VmaAllocation allocation)
	{
		std::scoped_lock lock(m_Mutex);

		Entry& entry = m_Entries.emplace_back();
		entry.AllocationHandle = allocation;
		entry.StructureType = type;
		entry.Handle = handle;
		entry.FrameNumber = Engine::FrameCounter;
	}

	VulkanSwapchain::VulkanSwapchain(uint32 width, uint32 height, PixelFormat desiredFormat, SDL_Window* window, VkQueue presentQueue, VulkanDevice* device)
		: m_Device(device)
		, m_WindowHandle(window)
		, m_PresentQueue(presentQueue)
	{
		m_Width = width;
		m_Height = height;
		m_Format = desiredFormat;

		CreateSwapchain();
	}

	void VulkanSwapchain::CreateSwapchain()
	{
		VkSurfaceCapabilitiesKHR capabilities;
		VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalHandle(), m_Surface, &capabilities));

		uint32 formatCount;
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &formatCount, nullptr));

		std::vector<VkSurfaceFormatKHR> swapchainFormats(formatCount);
		VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &formatCount, swapchainFormats.data()));

		uint32 presentModeCount;
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &presentModeCount, nullptr));

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);
		VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &presentModeCount, presentModes.data()));

		VkSurfaceFormatKHR surfaceFormat
		{
			.format = VulkanTools::ToVulkanFormat(m_Format),
			.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};

		// find best format
		{
			bool valid = false;
			for (const auto& format : swapchainFormats)
			{
				if (format.format == surfaceFormat.format)
				{
					valid = true;
					surfaceFormat.colorSpace = format.colorSpace;
					break;
				}
			}

			if (!valid)
			{
				surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
				surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
				m_Format = PixelFormat::BGRA8_UNORM;
			}
		}

		if (capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != UINT32_MAX)
		{
			m_Width = capabilities.currentExtent.width;
			m_Height = capabilities.currentExtent.height;
		}
		else
		{
			m_Width =
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, m_Width));
			m_Height =
				std::max(capabilities.minImageExtent.height, std::max(capabilities.maxImageExtent.height, m_Height));
		}

		uint32 numDesiredBuffers = std::max(FRAME_BUFFER_COUNT, capabilities.minImageCount);
		if ((capabilities.maxImageCount > 0) && (numDesiredBuffers > capabilities.maxImageCount))
		{
			numDesiredBuffers = capabilities.maxImageCount;
		}

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		if (!m_VSyncEnabled)
		{
			for (auto mode : presentModes)
			{
				if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					presentMode = mode;
					break;
				}
				else if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				{
					presentMode = mode;
					break;
				}
			}
		}

		VkSwapchainCreateInfoKHR swapchainInfo
		{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = m_Surface,
			.minImageCount = numDesiredBuffers,
			.imageFormat = surfaceFormat.format,
			.imageColorSpace = surfaceFormat.colorSpace,
			.imageExtent = VkExtent2D{m_Width, m_Height},
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = capabilities.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = presentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = VK_NULL_HANDLE
		};

		VK_CHECK(vkCreateSwapchainKHR(m_Device->GetDeviceHandle(), &swapchainInfo, nullptr, &m_Swapchain));

		uint32 numImages;
		VK_CHECK(vkGetSwapchainImagesKHR(m_Device->GetDeviceHandle(), m_Swapchain, &numImages, nullptr));

		std::vector<VkImage> images(numImages);
		VK_CHECK(vkGetSwapchainImagesKHR(m_Device->GetDeviceHandle(), m_Swapchain, &numImages, images.data()));

		m_Backbuffers.resize(numImages);
		for (uint32 i = 0; i < numImages; i++)
		{
			TextureDesc backBufferDesc
			{
				.Width = m_Width,
				.Height = m_Height,
				.Format = m_Format,
				.Dimension = TextureDimension::Texture2D
			};

			m_Backbuffers[i].Texture = new VulkanTexture(backBufferDesc, images[i], m_Device);

			VkSemaphoreCreateInfo semaphoreInfo
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};

			VK_CHECK(vkCreateSemaphore(m_Device->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_Backbuffers[i].RenderingDoneSemaphore));
		}

		m_AcquireSemaphores.resize(numDesiredBuffers);
		for (uint32 i = 0; i < numDesiredBuffers; i++)
		{
			VkSemaphoreCreateInfo semaphoreInfo
			{
				.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};

			VK_CHECK(vkCreateSemaphore(m_Device->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_AcquireSemaphores[i]);)
		}

		m_Outdated = false;
		m_ImageAcquired = false;
	}

	void VulkanSwapchain::CreateSurface()
	{
		if (m_Surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(m_Device->GetInstanceHandle(), m_Surface, nullptr);
			m_Surface = VK_NULL_HANDLE;
		}

		if (!SDL_Vulkan_CreateSurface(m_WindowHandle, m_Device->GetInstanceHandle(), nullptr, &m_Surface))
		{
			assert(false && "Failed to create Vulkan surface");
		}
	}

	void VulkanSwapchain::ReleaseSwapchainResources(bool destroySwapchain)
	{
		if (m_Swapchain == VK_NULL_HANDLE)
			return;

		VulkanCommandContext* cmdCxt = m_Device->MainContext;
		cmdCxt->FlushBarriers();
		cmdCxt->GetCmdManager().SubmitActiveCmd();

		m_Device->WaitGPUIdle();

		for (auto& backbuffer : m_Backbuffers)
		{
			vkDestroySemaphore(m_Device->GetDeviceHandle(), backbuffer.RenderingDoneSemaphore, nullptr);
		}

		for (auto& semaphore : m_AcquireSemaphores)
		{
			vkDestroySemaphore(m_Device->GetDeviceHandle(), semaphore, nullptr);
		}

		m_Backbuffers.clear();
		m_AcquireSemaphores.clear();
		m_Device->GetDeletionQueue().ReleaseResources(true);

		if (destroySwapchain)
		{
			vkDestroySwapchainKHR(m_Device->GetDeviceHandle(), m_Swapchain, nullptr);
			m_Swapchain = VK_NULL_HANDLE;
		}
	}

	void VulkanSwapchain::RecreateSwapchain()
	{
		// do not destroy swapchain as it will be used as oldSwapchain parameter
		ReleaseSwapchainResources(false);
		
		// check if surface is lost
		{
			VkSurfaceCapabilitiesKHR capabilites;
			VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalHandle(), m_Surface, &capabilites);
			if (err == VK_ERROR_SURFACE_LOST_KHR)
			{
				if (m_Swapchain != VK_NULL_HANDLE)
				{
					vkDestroySwapchainKHR(m_Device->GetDeviceHandle(), m_Swapchain, nullptr);
					m_Swapchain = nullptr;
				}

				// recreate surface
				CreateSurface();
			}
		}

		CreateSwapchain();
	}

	void VulkanSwapchain::Resize(uint32 width, uint32 height)
	{
		if (m_Width == width && m_Height == height)
			return;

		m_Width = width;
		m_Height = height;

		RecreateSwapchain();
	}

	VkResult VulkanSwapchain::AcquireImageIndex()
	{
		assert(!m_ImageAcquired);

		uint32 prevSemaphoreIndex = m_SemaphoreIndex;
		m_SemaphoreIndex = (m_SemaphoreIndex + 1) % (uint32)m_AcquireSemaphores.size();

		VkResult res = vkAcquireNextImageKHR(
			m_Device->GetDeviceHandle(),
			m_Swapchain,
			UINT64_MAX,
			m_AcquireSemaphores[m_SemaphoreIndex],
			VK_NULL_HANDLE,
			&m_AcquiredImageIndex);

		m_ImageAcquired = (res == VK_SUCCESS);
		if (m_ImageAcquired)
		{
			auto context = m_Device->MainContext;
			context->GetCmdManager().GetActive()->AddWaitSemaphore(m_AcquireSemaphores[m_SemaphoreIndex]);
		}
		else
		{
			m_SemaphoreIndex = prevSemaphoreIndex;
		}

		return res;
	}

	GPUTexture* VulkanSwapchain::GetBackBuffer()
	{
		if (!m_ImageAcquired)
		{
			VkResult res = m_Outdated ? VK_ERROR_OUT_OF_DATE_KHR : AcquireImageIndex();
			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
			{
				RecreateSwapchain();
				res = AcquireImageIndex();

				assert(res == VK_SUCCESS);
			}
		}

		return m_Backbuffers[m_AcquiredImageIndex].Texture;
	}

	void VulkanSwapchain::Present(bool vsync, bool isPrimary)
	{
		if (!m_ImageAcquired)
			return;

		const auto& backBuffer = m_Backbuffers[m_AcquiredImageIndex];
		auto        context = m_Device->MainContext;

		context->AddImageBarrier((VulkanTexture*)backBuffer.Texture, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		context->FlushBarriers();
		context->GetCmdManager().GetActive()->AddSignalSemaphore(backBuffer.RenderingDoneSemaphore);
		context->GetCmdManager().SubmitActiveCmd(isPrimary ? m_Device->FrameFences[Engine::FrameCounter % FRAME_BUFFER_COUNT] : VK_NULL_HANDLE);

		VkPresentInfoKHR presentInfo
		{
			.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &backBuffer.RenderingDoneSemaphore,
			.swapchainCount = 1,
			.pSwapchains = &m_Swapchain,
			.pImageIndices = &m_AcquiredImageIndex
		};

		VkResult res = vkQueuePresentKHR(m_PresentQueue, &presentInfo);
		if (res == VK_SUBOPTIMAL_KHR || 
			res == VK_ERROR_OUT_OF_DATE_KHR || 
			m_VSyncEnabled != vsync)
		{
			m_Outdated = true;
			m_VSyncEnabled = vsync;
		}

		m_ImageAcquired = false;
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VkCommandPool pool, VulkanDevice* device)
		: m_Device(device)
		, m_CmdPool(pool)
	{
		VkCommandBufferAllocateInfo cmdInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		VK_CHECK(vkAllocateCommandBuffers(device->GetDeviceHandle(), &cmdInfo, &m_CmdBuffer));
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		vkFreeCommandBuffers(m_Device->GetDeviceHandle(), m_CmdPool, 1, &m_CmdBuffer);
	}

	void VulkanCommandBuffer::AddWaitSemaphore(VkSemaphore semaphore, uint64 value, VkPipelineStageFlags2 stageFlags)
	{
		VkSemaphoreSubmitInfo semaphoreInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = semaphore,
			.value = value,
			.stageMask = stageFlags
		};

		WaitSemaphores.push_back(semaphoreInfo);
	}

	void VulkanCommandBuffer::AddSignalSemaphore(VkSemaphore semaphore, uint64 value, VkPipelineStageFlags2 stageFlags)
	{
		VkSemaphoreSubmitInfo semaphoreInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = semaphore,
			.value = value,
			.stageMask = stageFlags
		};

		SignalSemaphores.push_back(semaphoreInfo);
	}

	VulkanCmdBufferManager::VulkanCmdBufferManager(VkQueue queue, uint32 queueFamilyIndex, VulkanDevice* device)
		: m_Device(device)
		, m_Queue(queue)
	{
		VkCommandPoolCreateInfo poolInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queueFamilyIndex
		};

		VkSemaphoreCreateInfo semaphoreInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.flags = VK_SEMAPHORE_TYPE_TIMELINE
		};

		VK_CHECK(vkCreateCommandPool(device->GetDeviceHandle(), &poolInfo, nullptr, &m_Pool));
		VK_CHECK(vkCreateSemaphore(device->GetDeviceHandle(), &semaphoreInfo, nullptr, &m_TrackingSemaphore));
	}

	void VulkanCmdBufferManager::SubmitActiveCmd(VkFence fence)
	{
		if (m_ActiveCmdBuffer)
		{
			m_ActiveCmdBuffer->AddSignalSemaphore(m_TrackingSemaphore, m_SubmitCounter);
			m_ActiveCmdBuffer->SubmitID = ++m_SubmitCounter;

			VkCommandBufferSubmitInfo cmdInfo
			{
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
				.commandBuffer = m_ActiveCmdBuffer->GetCmdHandle()
			};

			VkSubmitInfo2 submitInfo
			{
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
				.waitSemaphoreInfoCount = (uint32)m_ActiveCmdBuffer->WaitSemaphores.size(),
				.pWaitSemaphoreInfos = m_ActiveCmdBuffer->WaitSemaphores.data(),
				.commandBufferInfoCount = 1,
				.pCommandBufferInfos = &cmdInfo,
				.signalSemaphoreInfoCount = (uint32)m_ActiveCmdBuffer->SignalSemaphores.size(),
				.pSignalSemaphoreInfos = m_ActiveCmdBuffer->WaitSemaphores.data()
			};

			VK_CHECK(vkQueueSubmit2(m_Queue, 1, &submitInfo, fence));

			m_ActiveCmdBuffer->WaitSemaphores.clear();
			m_ActiveCmdBuffer->SignalSemaphores.clear();
			m_CmdInFlight.push_back(m_ActiveCmdBuffer);
			m_ActiveCmdBuffer = nullptr;
		}
	}

	VulkanCommandBuffer* VulkanCmdBufferManager::GetActive()
	{
		if (!m_ActiveCmdBuffer)
		{
			// check if any of the submitted command buffers had finished execution
			if (m_FreeCmdBuffers.empty())
			{
				VK_CHECK(vkGetSemaphoreCounterValue(m_Device->GetDeviceHandle(), m_TrackingSemaphore, &m_LastFinishedID));

				while (!m_CmdInFlight.empty())
				{
					auto& cmd = m_CmdInFlight.front();
					if (cmd->SubmitID <= m_LastFinishedID)
					{
						cmd->SubmitID = 0;
						VK_CHECK(vkResetCommandBuffer(cmd->GetCmdHandle(), 0));
						m_FreeCmdBuffers.push_back(cmd);
					}
					else
					{
						break;
					}
				}
			}

			if (m_FreeCmdBuffers.empty())
			{
				m_ActiveCmdBuffer = CreateShared<VulkanCommandBuffer>(m_Pool, m_Device);
			}
			else
			{
				m_ActiveCmdBuffer = m_FreeCmdBuffers.back();
				m_FreeCmdBuffers.pop_back();
			}
		}

		return m_ActiveCmdBuffer.get();
	}

	void VulkanCommandContext::FlushBarriers()
	{
		VkDependencyInfo info{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		auto cmd = m_CmdBufferManager.GetActive();

		if (!m_ImageBarriers.empty())
		{
			info.imageMemoryBarrierCount = m_ImageBarriers.size();
			info.pImageMemoryBarriers = m_ImageBarriers.data();
		}
		if (!m_BufferBarriers.empty())
		{
			info.bufferMemoryBarrierCount = m_BufferBarriers.size();
			info.pBufferMemoryBarriers = m_BufferBarriers.data();
		}

		vkCmdPipelineBarrier2(cmd->GetCmdHandle(), &info);

		m_ImageBarriers.clear();
		m_BufferBarriers.clear();
	}

	void VulkanCommandContext::AddImageBarrier(VkImage image, VkImageLayout srcLayout, VkImageLayout dstLayout, const VkImageSubresourceRange& subresourceRange)
	{
		if (m_ImageBarriers.size() > 64)
		{
			FlushBarriers();
		}

		VkImageMemoryBarrier2& barrier = m_ImageBarriers.emplace_back();
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		barrier.oldLayout = srcLayout;
		barrier.newLayout = dstLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange = subresourceRange;

		VulkanTools::GetImageBarrierFlags(srcLayout, barrier.srcStageMask, barrier.srcAccessMask);
		VulkanTools::GetImageBarrierFlags(dstLayout, barrier.dstStageMask, barrier.dstAccessMask);
	}

	void VulkanCommandContext::AddImageBarrier(VulkanTexture* texture, uint32 mipSlice, uint32 arraySlice, VkImageLayout dstLayout)
	{
		uint32 subresourceIndex = mipSlice + arraySlice * texture->MipLevels();
		auto& state = texture->State;
		VkImageLayout srcLayout = state.GetSubresourceState(subresourceIndex);

		// TODO: maybe make UAV -> UAV barrier optional
		if (srcLayout == dstLayout 
			&& !(srcLayout == VK_IMAGE_LAYOUT_GENERAL && dstLayout == VK_IMAGE_LAYOUT_GENERAL))
			return;

		VkImageSubresourceRange range{};
		range.baseMipLevel = mipSlice;
		range.levelCount = 1;
		range.baseArrayLayer = arraySlice;
		range.layerCount = 1;
		range.aspectMask = texture->AspectFlags;

		AddImageBarrier(texture->GetImageHandle(), srcLayout, dstLayout, range);
		state.SetSubresourceState(subresourceIndex, dstLayout);
	}

	void VulkanCommandContext::AddImageBarrier(VulkanTexture* texture, VkImageLayout dstLayout)
	{
		auto& state = texture->State;
		if (state.AllSubresourcesSame)
		{
			VkImageLayout srcLayout = state.GetSubresourceState(0);
			if (srcLayout == dstLayout
				&& !(srcLayout == VK_IMAGE_LAYOUT_GENERAL && dstLayout == VK_IMAGE_LAYOUT_GENERAL))
				return;

			VkImageSubresourceRange range{};
			range.aspectMask = texture->AspectFlags;
			range.baseMipLevel = 0;
			range.levelCount = texture->MipLevels();
			range.layerCount = texture->ArraySize();
			range.baseArrayLayer = 0;

			AddImageBarrier(texture->GetImageHandle(), srcLayout, dstLayout, range);
			state.SetState(dstLayout);
		}
		else
		{
			for (uint32 arraySlice = 0; arraySlice < texture->ArraySize(); arraySlice++)
			{
				for (int32 mipSlice = 0; mipSlice < texture->MipLevels(); mipSlice++)
				{
					AddImageBarrier(texture, mipSlice, arraySlice, dstLayout);
				}
			}
		}
	}

	void VulkanCommandContext::AddImageBarrier(VulkanTextureView* view, VkImageLayout dstLayout)
	{
		VulkanTexture* vkOwner = static_cast<VulkanTexture*>(view->GetOwner());

		for (uint32 arraySlice = view->Subresources.baseArrayLayer; arraySlice < view->Subresources.layerCount; arraySlice++)
		{
			for (int32 mipSlice = view->Subresources.baseMipLevel; mipSlice < view->Subresources.levelCount; mipSlice++)
			{
				AddImageBarrier(vkOwner, mipSlice, arraySlice, dstLayout);
			}
		}
	}

	void VulkanCommandContext::AddBufferBarrier(VulkanBuffer* buffer, uint64 offset, uint64 size, VkAccessFlags2 dstAccess)
	{
		// skip if no transition requiered
		if ((buffer->Access & dstAccess) == dstAccess)
			return;

		if (m_BufferBarriers.size() > 64)
		{
			FlushBarriers();
		}

		VkBufferMemoryBarrier2& barrier = m_BufferBarriers.emplace_back();
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
		barrier.srcAccessMask = buffer->Access;
		barrier.dstAccessMask = dstAccess;
		barrier.srcStageMask = VulkanTools::GetBufferBarrierFlags(buffer->Access);
		barrier.dstStageMask = VulkanTools::GetBufferBarrierFlags(dstAccess);
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = buffer->GetBufferHandle();
		barrier.offset = offset;
		barrier.size = size;

		buffer->Access = dstAccess;
	}

	void VulkanCommandContext::AddBufferBarrier(VulkanBuffer* buffer, VkAccessFlags2 dstAccess)
	{
		AddBufferBarrier(buffer, 0, buffer->GetSize(), dstAccess);
	}

	void VulkanCommandContext::CopyTexture(GPUTexture* src, GPUTexture* dst, const TextureCopyInfo& info)
	{
		VulkanTexture* vkSrc = static_cast<VulkanTexture*>(src);
		VulkanTexture* vkDst = static_cast<VulkanTexture*>(dst);
		auto cmd = m_CmdBufferManager.GetActive();

		AddImageBarrier(vkSrc, info.SrcMipLevel, info.SrcSlice, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		AddImageBarrier(vkDst, info.DstMipLevel, info.DstSlice, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		FlushBarriers();

		VkImageCopy2 copy{ .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2 };
		copy.srcSubresource.aspectMask = vkSrc->AspectFlags;
		copy.srcSubresource.baseArrayLayer = info.SrcSlice;
		copy.srcSubresource.layerCount = 1;
		copy.srcSubresource.mipLevel = info.SrcMipLevel;
		copy.dstSubresource.aspectMask = vkDst->AspectFlags;
		copy.dstSubresource.baseArrayLayer = info.DstSlice;
		copy.dstSubresource.layerCount = 1;
		copy.dstSubresource.mipLevel = info.DstMipLevel;
		copy.srcOffset = VkOffset3D{ info.SrcOffset.x, info.SrcOffset.y, info.SrcOffset.z };
		copy.dstOffset = VkOffset3D{ info.DstOffset.x, info.DstOffset.y, info.DstOffset.z };
		copy.extent    = VkExtent3D{ info.CopySize.x, info.CopySize.y, info.CopySize.z };

		VkCopyImageInfo2 copyInfo{ .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2 };
		copyInfo.srcImage = vkSrc->GetImageHandle();
		copyInfo.dstImage = vkDst->GetImageHandle();
		copyInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copyInfo.regionCount = 1;
		copyInfo.pRegions = &copy;

		vkCmdCopyImage2(cmd->GetCmdHandle(), &copyInfo);
	}

	void VulkanCommandContext::CopyBuffer(GPUBuffer* src, uint64 srcOffset, GPUBuffer* dst, uint64 dstOffset, uint64 copySize)
	{
		VulkanBuffer* vkSrc = static_cast<VulkanBuffer*>(src);
		VulkanBuffer* vkDst = static_cast<VulkanBuffer*>(dst);
		auto cmd = m_CmdBufferManager.GetActive();

		AddBufferBarrier(vkSrc, srcOffset, copySize, VK_ACCESS_2_TRANSFER_READ_BIT);
		AddBufferBarrier(vkDst, dstOffset, copySize, VK_ACCESS_2_TRANSFER_WRITE_BIT);
		FlushBarriers();

		VkBufferCopy2 copy{ .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2 };
		copy.srcOffset = srcOffset;
		copy.dstOffset = dstOffset;
		copy.size = copySize;

		VkCopyBufferInfo2 copyInfo{ .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2 };
		copyInfo.srcBuffer = vkSrc->GetBufferHandle();
		copyInfo.dstBuffer = vkDst->GetBufferHandle();
		copyInfo.regionCount = 1;
		copyInfo.pRegions = &copy;

		vkCmdCopyBuffer2(cmd->GetCmdHandle(), &copyInfo);
	}

	void VulkanCommandContext::BindGraphicsState(GPUPipelineState* state)
	{
		VulkanPipelineState* vkState = static_cast<VulkanPipelineState*>(state);
		auto cmd = m_CmdBufferManager.GetActive();

		if (m_GraphicsState != vkState)
		{
			vkCmdBindPipeline(
				cmd->GetCmdHandle(),
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				vkState->GetPipelineHandle()
			);
			m_GraphicsState = vkState;
			m_DescriptorBinder.Dirty = true;
		}
	}

	void VulkanCommandContext::BindComputeState(GPUPipelineState* state)
	{
		VulkanPipelineState* vkState = static_cast<VulkanPipelineState*>(state);
		auto cmd = m_CmdBufferManager.GetActive();

		if (m_ComputeState != vkState)
		{
			vkCmdBindPipeline(
				cmd->GetCmdHandle(),
				VK_PIPELINE_BIND_POINT_COMPUTE,
				vkState->GetPipelineHandle()
			);
			m_ComputeState = vkState;
			m_DescriptorBinder.Dirty = true;
		}
	}

	VulkanDescriptorBinder::VulkanDescriptorBinder(VulkanDevice* device)
		: m_Device(device)
	{
		Writes.reserve(128);
		BufferInfos.reserve(128);
		ImageInfos.reserve(128);
	}

	void VulkanDescriptorBinder::InitPool(DescriptorPool& pool)
	{
		std::array<VkDescriptorPoolSize, 6> poolSizes =
		{
			VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = DESCRIPTOR_BINDER_CBV_COUNT * pool.Capacity},
			VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = DESCRIPTOR_BINDER_SRV_COUNT * pool.Capacity},
			VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = DESCRIPTOR_BINDER_SRV_COUNT * pool.Capacity},
			VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			.descriptorCount = DESCRIPTOR_BINDER_UAV_COUNT * pool.Capacity},
			VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = DESCRIPTOR_BINDER_UAV_COUNT * pool.Capacity},
			VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = DESCRIPTOR_BINDER_SAMPLER_COUNT * pool.Capacity}
		};

		VkDescriptorPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.poolSizeCount = (uint32)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = pool.Capacity;

		// issue destroy if already exists
		Destroy(pool);
		VK_CHECK(vkCreateDescriptorPool(m_Device->GetDeviceHandle(), &poolInfo, nullptr, &pool.Handle));
	}

	void VulkanDescriptorBinder::Destroy(DescriptorPool& pool)
	{
		if (pool.Handle != VK_NULL_HANDLE)
		{
			m_Device->GetDeletionQueue().Enqueue(
				VulkanDeletionQueue::Type::DescriptorPool,
				pool.Handle
			);

			pool.Handle = VK_NULL_HANDLE;
		}
	}

	void VulkanDescriptorBinder::Reset()
	{
		auto& pool = Pools[Engine::FrameCounter % FRAME_BUFFER_COUNT];
		if (pool.Handle != VK_NULL_HANDLE)
		{
			VK_CHECK(vkResetDescriptorPool(m_Device->GetDeviceHandle(), pool.Handle, 0));
		}

		DescriptorTable = {};
		Dirty = false;
	}

	void VulkanDescriptorBinder::FlushDescriptors(VulkanPipelineState* pipeline, bool graphics, VulkanCommandContext* context)
	{
		if (!Dirty)
			return;

		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = pipeline->GetSetLayoutHandle();

		const auto& descriptorInfo = pipeline->DescriptorInfo;
		auto cmd = context->GetCmdManager().GetActive();
		DescriptorPool& pool = Pools[Engine::FrameCounter % FRAME_BUFFER_COUNT];

		VkDescriptorSetAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = pool.Handle;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		VkResult res = vkAllocateDescriptorSets(m_Device->GetDeviceHandle(), &allocInfo, &descriptorSet);
		while (res == VK_ERROR_OUT_OF_POOL_MEMORY)
		{
			pool.Capacity *= 2;
			InitPool(pool);
			allocInfo.descriptorPool = pool.Handle;
			res = vkAllocateDescriptorSets(m_Device->GetDeviceHandle(), &allocInfo, &descriptorSet);
		}
		assert(res == VK_SUCCESS);

		Writes.clear();
		BufferInfos.clear();
		ImageInfos.clear();

		for (uint32 i = 0; i < descriptorInfo.DescriptorCount; i++)
		{
			auto& b = descriptorInfo.Descriptors[i];
			if (b.IsImmutable)
				continue;

			for (uint32 c = 0; c < b.Count; c++)
			{
				uint32 unrolledBinding = b.Binding + c;

				auto& write = Writes.emplace_back();
				write = {};
				write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				write.dstSet = descriptorSet;
				write.dstArrayElement = c;
				write.descriptorType = b.DescriptorType;
				write.dstBinding = b.Binding;
				write.descriptorCount = 1;

				switch (b.DescriptorType)
				{
				case VK_DESCRIPTOR_TYPE_SAMPLER:
				{
					auto& info = ImageInfos.emplace_back();
					write.pImageInfo = &info;
					info = {};

					uint32 originalBinding = unrolledBinding - VULKAN_BINDING_SHIFT_S;
					VulkanDescriptorResource* resource = DescriptorTable.Sampler[originalBinding];
					if (!resource)
					{
						info.sampler = m_Device->StaticSamplers[0];
					}
					else
					{
						resource->DescriptorAsSampler(info.sampler);
					}
				}
				break;

				case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				{
					auto& info = ImageInfos.emplace_back();
					write.pImageInfo = &info;
					info = {};

					uint32 originalBinding = unrolledBinding - VULKAN_BINDING_SHIFT_T;
					VulkanDescriptorResource* resource = DescriptorTable.SRV[originalBinding];
					if (!resource)
					{
						switch (b.Resource)
						{
						case SpirvDescriptorInfo::ResourceType::Texture2D:
							info.imageView = m_Device->NullImageView2D;
							break;
						case SpirvDescriptorInfo::ResourceType::Texture2DArray:
							info.imageView = m_Device->NullImageView2DArray;
							break;
						case SpirvDescriptorInfo::ResourceType::Texture3D:
							info.imageView = m_Device->NullImageView3D;
							break;
						case SpirvDescriptorInfo::ResourceType::TextureCube:
							info.imageView = m_Device->NullImageViewCube;
							break;
						default:
							break;
						}

						info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					}
					else
					{
						resource->DescriptorAsImage(context, info.imageView, info.imageLayout);
					}
				}
				break;

				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
				{
					auto& info = ImageInfos.emplace_back();
					write.pImageInfo = &info;
					info = {};

					uint32 originalBinding = unrolledBinding - VULKAN_BINDING_SHIFT_U;
					VulkanDescriptorResource* resource = DescriptorTable.UAV[originalBinding];
					if (!resource)
					{
						switch (b.Resource)
						{
						case SpirvDescriptorInfo::ResourceType::Texture2D:
							info.imageView = m_Device->NullImageView2D;
							break;
						case SpirvDescriptorInfo::ResourceType::Texture2DArray:
							info.imageView = m_Device->NullImageView2DArray;
							break;
						case SpirvDescriptorInfo::ResourceType::Texture3D:
							info.imageView = m_Device->NullImageView3D;
							break;
						case SpirvDescriptorInfo::ResourceType::TextureCube:
							info.imageView = m_Device->NullImageViewCube;
							break;
						default:
							break;
						}

						info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					}
					else
					{
						resource->DescriptorAsStorageImage(context, info.imageView, info.imageLayout);
					}
				}
				break;

				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				{
					auto& info = BufferInfos.emplace_back();
					write.pBufferInfo = &info;
					info = {};

					if (b.Type == SpirvDescriptorInfo::BindingType::SRV)
					{
						uint32 originalBinding = unrolledBinding - VULKAN_BINDING_SHIFT_T;
						VulkanDescriptorResource* resource = DescriptorTable.SRV[originalBinding];
						if (!resource)
						{
							info.buffer = m_Device->NullBuffer;
							info.offset = 0;
							info.range = VK_WHOLE_SIZE;
						}
						else
						{
							resource->DescriptorAsStorageBuffer(context, info.buffer, info.offset, info.range, true);
						}
					}
					else
					{
						uint32 originalBinding = unrolledBinding - VULKAN_BINDING_SHIFT_U;
						VulkanDescriptorResource* resource = DescriptorTable.UAV[originalBinding];
						if (!resource)
						{
							info.buffer = m_Device->NullBuffer;
							info.offset = 0;
							info.range = VK_WHOLE_SIZE;
						}
						else
						{
							resource->DescriptorAsStorageBuffer(context, info.buffer, info.offset, info.range);
						}
					}
				}
				break;

				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
				{
					auto& info = BufferInfos.emplace_back();
					write.pBufferInfo = &info;
					info = {};

					uint32 originalBinding = unrolledBinding - VULKAN_BINDING_SHIFT_B;
					VulkanDescriptorResource* resource = DescriptorTable.CBV[originalBinding];
					if (!resource)
					{
						info.buffer = m_Device->NullBuffer;
						info.offset = 0;
						info.range = VK_WHOLE_SIZE;
					}
					else
					{
						resource->DescriptorAsUniformBuffer(context, info.buffer, info.offset, info.range);
					}
				}
				break;

				default:
					assert(0);
					break;
				}
			}
		}

		vkUpdateDescriptorSets(
			m_Device->GetDeviceHandle(),
			(uint32)Writes.size(), 
			Writes.data(), 
			0, 
			nullptr);

		vkCmdBindDescriptorSets(
			cmd->GetCmdHandle(),
			graphics ? VK_PIPELINE_BIND_POINT_GRAPHICS : VK_PIPELINE_BIND_POINT_COMPUTE,
			pipeline->GetLayoutHandle(),
			0,
			1,
			& descriptorSet,
			0,
			nullptr
		);

		Dirty = false;
	}

	void VulkanCommandContext::BindCB(GPUBuffer* buffer, uint32 slot)
	{
		assert(slot < DESCRIPTOR_BINDER_CBV_COUNT);
		VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

		if (m_DescriptorBinder.DescriptorTable.CBV[slot] != vkBuffer)
		{
			m_DescriptorBinder.DescriptorTable.CBV[slot] = vkBuffer;
			m_DescriptorBinder.Dirty = true;
		}
	}

	void VulkanCommandContext::BindResource(GPUTextureView* view, uint32 slot)
	{
		assert(slot < DESCRIPTOR_BINDER_SRV_COUNT);
		VulkanTextureView* vkView = static_cast<VulkanTextureView*>(view);

		if (m_DescriptorBinder.DescriptorTable.SRV[slot] != vkView)
		{
			m_DescriptorBinder.DescriptorTable.SRV[slot] = vkView;
			m_DescriptorBinder.Dirty = true;
		}
	}

	void VulkanCommandContext::BindResource(GPUBuffer* buffer, uint32 slot)
	{
		assert(slot < DESCRIPTOR_BINDER_SRV_COUNT);
		VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

		if (m_DescriptorBinder.DescriptorTable.SRV[slot] != vkBuffer)
		{
			m_DescriptorBinder.DescriptorTable.SRV[slot] = vkBuffer;
			m_DescriptorBinder.Dirty = true;
		}
	}

	void VulkanCommandContext::BindUAV(GPUTextureView* view, uint32 slot)
	{
		assert(slot < DESCRIPTOR_BINDER_UAV_COUNT);
		VulkanTextureView* vkView = static_cast<VulkanTextureView*>(view);

		if (m_DescriptorBinder.DescriptorTable.UAV[slot] != vkView)
		{
			m_DescriptorBinder.DescriptorTable.UAV[slot] = vkView;
			m_DescriptorBinder.Dirty = true;
		}
	}

	void VulkanCommandContext::BindUAV(GPUBuffer* buffer, uint32 slot)
	{
		assert(slot < DESCRIPTOR_BINDER_UAV_COUNT);
		VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);

		if (m_DescriptorBinder.DescriptorTable.UAV[slot] != vkBuffer)
		{
			m_DescriptorBinder.DescriptorTable.UAV[slot] = vkBuffer;
			m_DescriptorBinder.Dirty = true;
		}
	}

	void VulkanCommandContext::BindSamplerState(GPUSamplerState* sampler, uint32 slot)
	{
		assert(slot < DESCRIPTOR_BINDER_SAMPLER_COUNT);
		VulkanSamplerState* vkSampler = static_cast<VulkanSamplerState*>(sampler);

		if (m_DescriptorBinder.DescriptorTable.Sampler[slot] != vkSampler)
		{
			m_DescriptorBinder.DescriptorTable.Sampler[slot] = vkSampler;
			m_DescriptorBinder.Dirty = true;
		}
	}

	VulkanBuffer::VulkanBuffer(const BufferDesc& desc, VulkanDevice& device)
		: GPUBuffer(desc)
		, m_Device(device)
		, m_MappedData(nullptr)
	{
		VkBufferCreateInfo bufferInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = desc.ByteSize;
		bufferInfo.usage = 0;

		if (EnumHasAllFlags(desc.Flags, BufferFlags::ShaderResource))
			bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (EnumHasAllFlags(desc.Flags, BufferFlags::IndexBuffer))
			bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		if (EnumHasAllFlags(desc.Flags, BufferFlags::VertexBuffer))
			bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		if (EnumHasAllFlags(desc.Flags, BufferFlags::Indirect))
			bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (EnumHasAllFlags(desc.Flags, BufferFlags::Storage))
			bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (EnumHasAllFlags(desc.Flags, BufferFlags::GPUAddress))
			bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		if (EnumHasAllFlags(desc.Flags, BufferFlags::ReadBack))
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}
		else if (EnumHasAllFlags(desc.Flags, BufferFlags::Upload))
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		}
		else 
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		}

		VK_CHECK(vmaCreateBuffer(m_Device.GetAllocatorHandle(), &bufferInfo, &allocInfo, &m_Buffer,
			&m_Allocation, nullptr));

		if (EnumHasAnyFlags(desc.Flags, BufferFlags::Upload | BufferFlags::ReadBack))
		{
			vmaMapMemory(m_Device.GetAllocatorHandle(), m_Allocation, &m_MappedData);
		}
	}

	void VulkanBuffer::DescriptorAsStorageBuffer(VulkanCommandContext* context, VkBuffer& buffer, VkDeviceSize& offset, VkDeviceSize& range, bool isSRV)
	{
		buffer = m_Buffer;
		offset = 0;
		range = m_Desc.ByteSize;

		VkAccessFlags2 access = isSRV ? VK_ACCESS_2_SHADER_READ_BIT : VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
		context->AddBufferBarrier(this, access);
	}

	void VulkanBuffer::DescriptorAsUniformBuffer(VulkanCommandContext* context, VkBuffer& buffer, VkDeviceSize& offset, VkDeviceSize& range)
	{
		buffer = m_Buffer;
		offset = 0;
		range = m_Desc.ByteSize;

		context->AddBufferBarrier(this, VK_ACCESS_2_SHADER_READ_BIT);
	}

	VulkanBuffer::~VulkanBuffer() 
	{
		m_Device.GetDeletionQueue().Enqueue(
			VulkanDeletionQueue::Type::Buffer,
			m_Buffer,
			m_Allocation
		);
	}

	VulkanTexture::VulkanTexture(const TextureDesc& desc, VulkanDevice* device)
		: GPUTexture(desc)
		, m_Device(device)
	{
		State.Initialize(desc.MipLevels * desc.ArraySize, VK_IMAGE_LAYOUT_UNDEFINED, true);
		if (PixelFormatInfo::HasDepth(desc.Format))
		{
			AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (PixelFormatInfo::HasStencil(desc.Format))
			{
				AspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkImageCreateInfo imgInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imgInfo.format = VulkanTools::ToVulkanFormat(desc.Format);
		imgInfo.extent = VkExtent3D{ desc.Width, desc.Height, desc.Depth };
		imgInfo.mipLevels = desc.MipLevels;
		imgInfo.arrayLayers = desc.ArraySize;
		imgInfo.samples = (VkSampleCountFlagBits)desc.SampleCount;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imgInfo.flags = 0;

		switch (desc.Dimension)
		{
		case TextureDimension::Texture2D:
			imgInfo.imageType = VK_IMAGE_TYPE_2D;
		case TextureDimension::TextureCube:
			imgInfo.imageType = VK_IMAGE_TYPE_2D;
			imgInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		case TextureDimension::Texture3D:
			imgInfo.imageType = VK_IMAGE_TYPE_3D;
		default:
			assert(0);
			break;
		}

		if (EnumHasAllFlags(desc.Flags, TextureFlags::ShaderResource))
			imgInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (EnumHasAllFlags(desc.Flags, TextureFlags::UnorderedAccess))
			imgInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (EnumHasAllFlags(desc.Flags, TextureFlags::RenderTarget))
			imgInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (EnumHasAllFlags(desc.Flags, TextureFlags::DepthStencil))
			imgInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VK_CHECK(vmaCreateImage(m_Device->GetAllocatorHandle(), &imgInfo, &allocInfo, &m_Image, &m_Allocation, nullptr));

		InitViews();
	}

	VulkanTexture::VulkanTexture(const TextureDesc& desc, VkImage image, VulkanDevice* device)
		: GPUTexture(desc)
		, m_Device(device)
		, m_Image(image)
		, m_Allocation(nullptr)
	{
		State.Initialize(desc.MipLevels * desc.ArraySize, VK_IMAGE_LAYOUT_UNDEFINED, true);
		if (PixelFormatInfo::HasDepth(desc.Format))
		{
			AspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (PixelFormatInfo::HasStencil(desc.Format))
			{
				AspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else
		{
			AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		InitViews();
	}

	VulkanTexture::~VulkanTexture() 
	{
		if (m_Allocation)
		{
			m_Device->GetDeletionQueue().Enqueue(
				VulkanDeletionQueue::Type::Image,
				m_Image,
				m_Allocation
			);
		}

		m_ViewArray.Release();
		m_ViewVolume.Release();
		m_ViewReadOnlyDepth.Release();
		m_ViewStencil.Release();
		
		for (int32 i = 0; i < m_ViewsPerMip.size(); i++)
		{
			for (int32 j = 0; j < m_ViewsPerMip[i].size(); j++)
			{
				m_ViewsPerMip[i][j].Release();
			}
		}

		for (int32 i = 0; i < m_ViewsPerSlice.size(); i++)
		{
			m_ViewsPerSlice[i].Release();
		}

		m_ViewsPerMip.clear();
		m_ViewsPerSlice.clear();
	}

	void VulkanTexture::InitViews()
	{
		if (EnumHasAllFlags(m_Desc.Flags, TextureFlags::StreamedTexture))
		{
			// for streamed texture view will be recreated as new mips upload
			m_ViewsPerSlice.resize(1);
		}
		else
		{
			bool isVolume = m_Desc.Dimension == TextureDimension::Texture3D;
			bool isArray = m_Desc.ArraySize > 1;
			bool isCubeMap = m_Desc.Dimension == TextureDimension::TextureCube;

			if (isVolume)
			{
				m_ViewVolume.Init(m_Device, this, VK_IMAGE_VIEW_TYPE_3D, m_Desc.Format, 0, m_Desc.MipLevels, 0, m_Desc.ArraySize);

				if (EnumHasAllFlags(m_Desc.Flags, TextureFlags::PerSliceViews))
				{
					m_ViewsPerSlice.resize(m_Desc.Depth);
					for (int32 slice = 0; slice < m_Desc.Depth; slice++)
					{
						m_ViewsPerSlice[slice].Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D, m_Desc.Format, 0, m_Desc.MipLevels, slice, 1);
					}
				}
			}
			else if (isArray)
			{
				m_ViewsPerSlice.resize(m_Desc.ArraySize);

				for (int32 slice = 0; slice < m_Desc.ArraySize; slice++)
				{
					m_ViewsPerSlice[slice].Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D, m_Desc.Format, 0, m_Desc.MipLevels, slice, 1);
				}

				if (isCubeMap)
				{
					m_ViewArray.Init(m_Device, this, VK_IMAGE_VIEW_TYPE_CUBE, m_Desc.Format, 0, m_Desc.MipLevels, 0, m_Desc.ArraySize);
				}
				else
				{
					m_ViewArray.Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D_ARRAY, m_Desc.Format, 0, m_Desc.MipLevels, 0, m_Desc.ArraySize);
				}
			}
			else
			{
				m_ViewsPerSlice.resize(1);
				if (isCubeMap)
				{
					m_ViewsPerSlice[0].Init(m_Device, this, VK_IMAGE_VIEW_TYPE_CUBE, m_Desc.Format, 0, m_Desc.MipLevels, 0, m_Desc.ArraySize);
				}
				else
				{
					m_ViewsPerSlice[0].Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D, m_Desc.Format, 0, m_Desc.MipLevels, 0, 1);
				}
			}

			if (EnumHasAllFlags(m_Desc.Flags, TextureFlags::PerMipViews))
			{
				m_ViewsPerMip.resize(m_Desc.ArraySize);
				for (int32 arrayIndex = 0; arrayIndex < m_Desc.ArraySize; arrayIndex++)
				{
					auto& slice = m_ViewsPerMip[arrayIndex];
					slice.resize(m_Desc.MipLevels);

					for (int32 mip = 0; mip < m_Desc.MipLevels; mip++)
					{
						slice[mip].Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D, m_Desc.Format, mip, 1, arrayIndex, 1);
					}
				}
			}

			if (EnumHasAllFlags(m_Desc.Flags, TextureFlags::DepthStencil))
			{
				m_ViewReadOnlyDepth.Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D, m_Desc.Format, 0, m_Desc.MipLevels, 0, 1, true);

				if (EnumHasAllFlags(m_Desc.Flags, TextureFlags::ShaderResource) && PixelFormatInfo::HasStencil(m_Desc.Format))
				{
					PixelFormat stencilFormat;
					switch (m_Desc.Format)
					{
					case PixelFormat::D24S8:
						stencilFormat = PixelFormat::X24G8_UINT;
						break;
					case PixelFormat::D32S8:
						stencilFormat = PixelFormat::X32G8_UINT;
						break;
					}

					m_ViewStencil.Init(m_Device, this, VK_IMAGE_VIEW_TYPE_2D, stencilFormat, 0, m_Desc.MipLevels, 0, 1, true, true);
				}
			}
		}
	}

	void VulkanTextureView::Init(VulkanDevice* device, VulkanTexture* owner, VkImageViewType viewType, PixelFormat format,
		uint32 baseMip, uint32 mipLevels, uint32 baseLayer, uint32 numLayers, bool readOnlyDepth, bool stencilView)
	{
		ViewType = viewType;
		Device = device;

		GPUTextureView::Init(owner, format);

		VkImageViewCreateInfo info{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		info.image = owner->GetImageHandle();
		info.viewType = viewType;
		info.format = VulkanTools::ToVulkanFormat(format);

		Subresources.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		Subresources.baseMipLevel = baseMip;
		Subresources.levelCount = mipLevels;
		Subresources.baseArrayLayer = baseLayer;
		Subresources.layerCount = numLayers;

		if (stencilView)
		{
			Subresources.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
			LayoutRTV = readOnlyDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			LayoutSRV = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else if (PixelFormatInfo::HasDepth(format))
		{
			Subresources.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (PixelFormatInfo::HasStencil(format))
				Subresources.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

			LayoutRTV = readOnlyDepth ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			LayoutSRV = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		}
		else
		{
			LayoutRTV = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			LayoutSRV = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}

		info.subresourceRange = Subresources;
		VK_CHECK(vkCreateImageView(device->GetDeviceHandle(), &info, nullptr, &View));
	}

	void VulkanTextureView::DescriptorAsImage(VulkanCommandContext* context, VkImageView& view, VkImageLayout& layout)
	{
		view = View;
		layout = LayoutSRV;

		VkImageAspectFlags aspect = Subresources.aspectMask;
		if (aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		{
			if (PartialView == nullptr)
			{
				VkImageViewCreateInfo createInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				createInfo.image = (static_cast<VulkanTexture*>(m_Owner))->GetImageHandle();
				createInfo.viewType = ViewType;
				createInfo.format = VulkanTools::ToVulkanFormat(m_Format);
				createInfo.subresourceRange = Subresources;

				if (aspect == VK_IMAGE_ASPECT_STENCIL_BIT)
				{
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
					createInfo.components.g = VK_COMPONENT_SWIZZLE_R;
				}
				else
				{
					createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
				}

				VK_CHECK(vkCreateImageView(Device->GetDeviceHandle(), &createInfo, nullptr, &PartialView))
			}
			view = PartialView;
		}

		context->AddImageBarrier(this, LayoutSRV);
	}

	void VulkanTextureView::DescriptorAsStorageImage(VulkanCommandContext* context, VkImageView& view, VkImageLayout& layout)
	{
		view = View;
		layout = VK_IMAGE_LAYOUT_GENERAL;
		context->AddImageBarrier(this, VK_IMAGE_LAYOUT_GENERAL);
	}

	void VulkanTextureView::Release()
	{
		if (View != VK_NULL_HANDLE)
		{
			Device->GetDeletionQueue().Enqueue(
				VulkanDeletionQueue::Type::ImageView,
				View
			);
		}

		if (PartialView != VK_NULL_HANDLE)
		{
			Device->GetDeletionQueue().Enqueue(
				VulkanDeletionQueue::Type::ImageView,
				PartialView
			);
		}

		View = VK_NULL_HANDLE;
		PartialView = VK_NULL_HANDLE;
	}

	VulkanSamplerState::VulkanSamplerState(const SamplerStateDesc& desc, VulkanDevice* device) 
		: GPUSamplerState(desc)
		, m_Device(device)
	{
		VkSamplerCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

		switch (desc.Filter)
		{
		case SamplerFilter::Point:
			info.magFilter = VK_FILTER_NEAREST;
			info.minFilter = VK_FILTER_NEAREST;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case SamplerFilter::Bilinear:
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		case SamplerFilter::Trilinear:
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case SamplerFilter::Anisotropic:
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			info.anisotropyEnable = true;
			info.maxAnisotropy = std::min(device->GetLimits().maxSamplerAnisotropy, std::max(1.f, (float)desc.MaxAnisotropy));
			break;
		default:
			info.magFilter = VK_FILTER_NEAREST;
			info.minFilter = VK_FILTER_NEAREST;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}

		info.addressModeU = VulkanTools::ToVulkanAddressMode(desc.AddressU);
		info.addressModeV = VulkanTools::ToVulkanAddressMode(desc.AddressV);
		info.addressModeW = VulkanTools::ToVulkanAddressMode(desc.AddressW);
		info.minLod = desc.MinMipLevel;
		info.maxLod = desc.MaxMipLevel;

		VK_CHECK(vkCreateSampler(m_Device->GetDeviceHandle(), &info, nullptr, &m_Sampler));
	}

	VulkanSamplerState::~VulkanSamplerState() 
	{
		m_Device->GetDeletionQueue().Enqueue(
			VulkanDeletionQueue::Type::Sampler,
			m_Sampler
		);
	}

	VulkanShader::VulkanShader(std::span<uint8> bytecode, ShaderStage stage, VulkanDevice* device)
		: m_Device(device)
		, m_Stage(stage)
	{
		// header comes first, later the spirv code
		SpirvDescriptorInfo* header = (SpirvDescriptorInfo*)bytecode.data();
		DescriptorInfo = *header;
		bytecode = bytecode.subspan(sizeof(SpirvDescriptorInfo));

		VkShaderModuleCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		info.codeSize = bytecode.size();
		info.pCode = (uint32*)bytecode.data();

		VK_CHECK(vkCreateShaderModule(m_Device->GetDeviceHandle(), &info, nullptr, &m_Module));

		switch (stage)
		{
		case ShaderStage::Vertex:
			StageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case ShaderStage::Pixel:
			StageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case ShaderStage::Compute:
			StageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		default:
			assert(0);
			break;
		}
	}

	VulkanShader::~VulkanShader() 
	{
		vkDestroyShaderModule(m_Device->GetDeviceHandle(), m_Module, nullptr);
	}

	VulkanPipelineState::VulkanPipelineState(const PipelineStateDesc& desc, VulkanDevice* device)
		: m_Device(device)
	{
		VulkanPSOLayoutHash layoutHash = {};
		{
			auto insertShader = [&, this](GPUShader* shader) {
				if (!shader)
					return;

				VulkanShader* vkShader = static_cast<VulkanShader*>(vkShader);
				auto&         descriptorInfo = vkShader->DescriptorInfo;

				for (uint32 i = 0; i < descriptorInfo.DescriptorCount; i++)
				{
					auto& b = descriptorInfo.Descriptors[i];
					bool found = false;

					for (uint32 m = 0; m < DescriptorInfo.DescriptorCount; m++)
					{
						auto& b2 = DescriptorInfo.Descriptors[i];
						if (b.Binding == b2.Binding)
						{
							// check for overlapping descriptors
							assert(b.Count == b2.Count
								&& b.Type == b2.Type);

							layoutHash.Bindings[i].stageFlags |= vkShader->StageFlags;
							found = true;
							break;
						}
					}

					if (!found)
					{
						assert(DescriptorInfo.DescriptorCount < SpirvDescriptorInfo::MaxDescriptors);
						DescriptorInfo.Descriptors[DescriptorInfo.DescriptorCount] = b;
						DescriptorInfo.DescriptorCount++;

						auto& vkBinding = layoutHash.Bindings.emplace_back();
						vkBinding.binding = b.Binding;
						vkBinding.descriptorCount = b.Count;
						vkBinding.descriptorType = b.DescriptorType;
						vkBinding.stageFlags = vkShader->StageFlags;

						if (b.IsImmutable && b.DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
						{
							uint32 samplerOffset = b.Binding - VULKAN_BINDING_SHIFT_S - VULKAN_IMMUTABLE_SAMPLER_FIRST_BINDING;
							vkBinding.pImmutableSamplers = device->StaticSamplers + samplerOffset;
						}
						else
						{
							vkBinding.pImmutableSamplers = nullptr;
						}
					}
				}

				if (descriptorInfo.PushConstantSize > 0)
				{
					DescriptorInfo.PushConstantSize = std::max(DescriptorInfo.PushConstantSize, descriptorInfo.PushConstantSize);
					DescriptorInfo.PushConstantOffset = std::max(DescriptorInfo.PushConstantOffset, descriptorInfo.PushConstantOffset);

					layoutHash.PushConstants.size = DescriptorInfo.PushConstantSize;
					layoutHash.PushConstants.offset = DescriptorInfo.PushConstantOffset;
					layoutHash.PushConstants.stageFlags |= vkShader->StageFlags;
				}
				};

			insertShader(desc.ComputeShader);
			insertShader(desc.VertexShader);
			insertShader(desc.PixelShader);
		}
		{
			VulkanPSOLayout layout = m_Device->CreateCachedPSOLayout(layoutHash);
			m_Layout = layout.Layout;
			m_SetLayout = layout.SetLayout;
		}

		if (desc.ComputeShader) 
		{
			VkPipelineShaderStageCreateInfo stageInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			stageInfo.module = (VkShaderModule)desc.ComputeShader->GetNative();
			stageInfo.pName = "CSMain";

			VkComputePipelineCreateInfo info{ .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
			info.stage = stageInfo;
			info.layout = m_Layout;

			VK_CHECK(vkCreateComputePipelines(m_Device->GetDeviceHandle(), m_Device->GetPipelineCacheHandle(),
				1, &info, nullptr, &m_Pipeline));
		}
		else 
		{
			VkPipelineShaderStageCreateInfo vertexShader{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertexShader.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertexShader.module = desc.VertexShader ? (VkShaderModule)desc.VertexShader->GetNative() : nullptr;
			vertexShader.pName = "VSMain";

			VkPipelineShaderStageCreateInfo pixelShader{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			pixelShader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			pixelShader.module = desc.PixelShader ? (VkShaderModule)desc.PixelShader->GetNative() : nullptr;
			pixelShader.pName = "PSMain";

			VkPipelineShaderStageCreateInfo shaders[2] = { vertexShader, pixelShader };

			VkPipelineInputAssemblyStateCreateInfo inputInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
			inputInfo.primitiveRestartEnable = false;

			switch (desc.PrimitiveTopology)
			{
			case PrimitiveTopology::PointList:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
				break;
			case PrimitiveTopology::LineList:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				break;
			case PrimitiveTopology::LineStrip:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
				break;
			case PrimitiveTopology::TriangleList:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				break;
			case PrimitiveTopology::TriangleStrip:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				break;
			case PrimitiveTopology::TriangleFan:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
				break;
			case PrimitiveTopology::TriangleListWithAdjacency:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
				break;
			case PrimitiveTopology::TriangleStripWithAdjacency:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
				break;
			case PrimitiveTopology::PatchList:
				inputInfo.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
				break;
			default:
				assert(0);
				break;
			}

			VkPipelineRasterizationStateCreateInfo rasterInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterInfo.polygonMode = desc.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
			rasterInfo.lineWidth = 1.f;

			switch (desc.CullMode)
			{
			case CullMode::FrontFace:
				rasterInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
				break;
			case CullMode::BackFace:
				rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
				break;
			case CullMode::None:
			default:
				rasterInfo.cullMode = VK_CULL_MODE_NONE;
				break;
			}

			switch (desc.FrontFace)
			{
			case FrontFace::CounterClockWise:
				rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
				break;
			case FrontFace::ClockWise:
			default:
				rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
				break;
			}

			VkFormat                            rtFormats[8] = {};
			VkPipelineColorBlendAttachmentState blendStates[8] = {};

			for (uint8 i = 0; i < desc.NumRenderTargets; i++) 
			{
				auto& rt = desc.RenderTargets[i];

				blendStates[i].blendEnable = rt.EnableBlend;
				blendStates[i].srcColorBlendFactor = VulkanTools::ToVulkanBlendFactor(rt.SrcBlend);
				blendStates[i].dstColorBlendFactor = VulkanTools::ToVulkanBlendFactor(rt.DstBlend);
				blendStates[i].colorBlendOp = VulkanTools::ToVulkanBlendOp(rt.BlendOp);
				blendStates[i].srcAlphaBlendFactor = VulkanTools::ToVulkanBlendFactor(rt.SrcBlendAlpha);
				blendStates[i].dstAlphaBlendFactor = VulkanTools::ToVulkanBlendFactor(rt.DstBlendAlpha);
				blendStates[i].alphaBlendOp = VulkanTools::ToVulkanBlendOp(rt.BlendOpAlpha);
				blendStates[i].colorWriteMask = (uint32)rt.ColorMask;

				rtFormats[i] = VulkanTools::ToVulkanFormat(rt.Format);
			}

			VkPipelineColorBlendStateCreateInfo blendingInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			blendingInfo.logicOpEnable = false;
			blendingInfo.logicOp = VK_LOGIC_OP_COPY;
			blendingInfo.attachmentCount = desc.NumRenderTargets;
			blendingInfo.pAttachments = blendStates;

			VkPipelineVertexInputStateCreateInfo vertexInputInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

			VkPipelineRenderingCreateInfo renderingInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
			renderingInfo.colorAttachmentCount = desc.NumRenderTargets;
			renderingInfo.pColorAttachmentFormats = rtFormats;
			renderingInfo.depthAttachmentFormat = VulkanTools::ToVulkanFormat(desc.DepthFormat);

			// TODO: add stencil support
			renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			depthStencilInfo.depthTestEnable = desc.DepthEnable;
			depthStencilInfo.depthWriteEnable = desc.DepthWriteEnable;
			depthStencilInfo.depthCompareOp = VulkanTools::ToVulkanCompareOp(desc.DepthFunc);
			depthStencilInfo.depthBoundsTestEnable = desc.DepthClipEnable;
			depthStencilInfo.stencilTestEnable = desc.StencilEnable;
			
			if (desc.StencilEnable) 
			{
				depthStencilInfo.front.failOp = VulkanTools::ToVulkanStencilOp(desc.FrontStencil.FailOp);
				depthStencilInfo.front.passOp = VulkanTools::ToVulkanStencilOp(desc.FrontStencil.PassOp);
				depthStencilInfo.front.depthFailOp = VulkanTools::ToVulkanStencilOp(desc.FrontStencil.DepthFailOp);
				depthStencilInfo.front.compareOp = VulkanTools::ToVulkanCompareOp(desc.FrontStencil.Func);
				depthStencilInfo.front.compareMask = desc.FrontStencil.ReadMask;
				depthStencilInfo.front.writeMask = desc.FrontStencil.WriteMask;
				depthStencilInfo.front.reference = 0;

				depthStencilInfo.back.failOp = VulkanTools::ToVulkanStencilOp(desc.BackStencil.FailOp);
				depthStencilInfo.back.passOp = VulkanTools::ToVulkanStencilOp(desc.BackStencil.PassOp);
				depthStencilInfo.back.depthFailOp = VulkanTools::ToVulkanStencilOp(desc.BackStencil.DepthFailOp);
				depthStencilInfo.back.compareOp = VulkanTools::ToVulkanCompareOp(desc.BackStencil.Func);
				depthStencilInfo.back.compareMask = desc.BackStencil.ReadMask;
				depthStencilInfo.back.writeMask = desc.BackStencil.WriteMask;
				depthStencilInfo.back.reference = 0;
			}
			else 
			{
				depthStencilInfo.front.compareMask = 0;
				depthStencilInfo.front.writeMask = 0;
				depthStencilInfo.front.reference = 0;
				depthStencilInfo.front.compareOp = VK_COMPARE_OP_NEVER;
				depthStencilInfo.front.passOp = VK_STENCIL_OP_KEEP;
				depthStencilInfo.front.failOp = VK_STENCIL_OP_KEEP;
				depthStencilInfo.front.depthFailOp = VK_STENCIL_OP_KEEP;

				depthStencilInfo.back.compareMask = 0;
				depthStencilInfo.back.writeMask = 0;
				depthStencilInfo.back.reference = 0;
				depthStencilInfo.back.compareOp = VK_COMPARE_OP_NEVER;
				depthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
				depthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
				depthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
			}


			VkPipelineMultisampleStateCreateInfo multisamplingInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			// TODO: add support for MSAA
			multisamplingInfo.sampleShadingEnable = false;
			multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisamplingInfo.minSampleShading = 1.0f;
			multisamplingInfo.pSampleMask = nullptr;
			multisamplingInfo.alphaToCoverageEnable = false;
			multisamplingInfo.alphaToOneEnable = false;

			VkPipelineViewportStateCreateInfo viewportInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportInfo.viewportCount = 1;
			viewportInfo.scissorCount = 1;

			VkGraphicsPipelineCreateInfo pipelineInfo = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
			pipelineInfo.pNext = &renderingInfo;
			pipelineInfo.stageCount = 2;
			pipelineInfo.pStages = shaders;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &inputInfo;
			pipelineInfo.pViewportState = &viewportInfo;
			pipelineInfo.pRasterizationState = &rasterInfo;
			pipelineInfo.pMultisampleState = &multisamplingInfo;
			pipelineInfo.pColorBlendState = &blendingInfo;
			pipelineInfo.pDepthStencilState = &depthStencilInfo;
			pipelineInfo.layout = m_Layout;

			VkDynamicState dynStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamicInfo.pDynamicStates = dynStates;
			dynamicInfo.dynamicStateCount = 2;

			pipelineInfo.pDynamicState = &dynamicInfo;

			VK_CHECK(vkCreateGraphicsPipelines(m_Device->GetDeviceHandle(), m_Device->GetPipelineCacheHandle(), 1, &pipelineInfo, nullptr, &m_Pipeline));
		}
	}

	VulkanPipelineState::~VulkanPipelineState() 
	{
		m_Device->GetDeletionQueue().Enqueue(
			VulkanDeletionQueue::Type::Pipeline,
			m_Pipeline
		);
	}
}