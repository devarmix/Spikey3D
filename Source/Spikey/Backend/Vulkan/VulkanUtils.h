#pragma once

#include <Engine/Graphics/GraphicsCore.h>
#include <Backend/Vulkan/VulkanCommon.h>

namespace Spikey::VulkanUtils {

	VkFormat TextureFormatToVulkan(ETextureFormat format);
	VkImageAspectFlags TextureFormatToAspect(ETextureFormat format);
	VkImageUsageFlags TextureUsageToVulkan(ETextureUsage flags);
	VkImageLayout GPUAccessToVulkanLayout(EGPUAccess flags);
	VkAccessFlags2 GPUAccessToVulkanAccess(EGPUAccess flags);
	VkPipelineStageFlags2 GPUAccessToVulkanStage(EGPUAccess flags);
	VkBufferUsageFlags BufferUsageToVulkan(EBufferUsage flags);
	VkDescriptorType BindingTypeToVulkan(EShaderResourceType type);
	VkFrontFace FrontFaceToVulkan(EFrontFace face);
	VkFilter SamplerFilterToVulkan(ESamplerFilter filter);
	VkSamplerMipmapMode SamplerMipMapModeToVulkan(ESamplerFilter filter);
	VkSamplerAddressMode SamplerAddressToVulkan(ESamplerAddress address);
	VkSamplerReductionMode SamplerReductionToVulkan(ESamplerReduction reduction);
	VkImageSubresourceRange SubresourceRangeToVulkan(const SubresourceRange& range);
}