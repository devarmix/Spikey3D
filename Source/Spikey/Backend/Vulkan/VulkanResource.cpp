#include <Backend/Vulkan/VulkanRHI.h>
#include <Backend/Vulkan/VulkanResource.h>

#include <spirv_reflect.h>

namespace Spikey {

	constexpr VkFormat ConvertImageFormat(ETextureFormat format) {
		switch (format)
		{
		case ETextureFormat::RGBA8U:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case ETextureFormat::BGRA8U:
			return VK_FORMAT_B8G8R8A8_UNORM;
		case ETextureFormat::RGBA16F:
			return VK_FORMAT_R16G16B16A16_SFLOAT;
		case ETextureFormat::RGBA16U:
			return VK_FORMAT_R16G16B16A16_UNORM;
		case ETextureFormat::RGBA32F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case ETextureFormat::D32F:
			return VK_FORMAT_D32_SFLOAT;
		case ETextureFormat::R32F:
			return VK_FORMAT_R32_SFLOAT;
		case ETextureFormat::RG16F:
			return VK_FORMAT_R16G16_SFLOAT;
		case ETextureFormat::RG16U:
			return VK_FORMAT_R16G16_UNORM;
		case ETextureFormat::RG8U:
			return VK_FORMAT_R8G8_UNORM;
		case ETextureFormat::R8U:
			return VK_FORMAT_R8_UNORM;
		case ETextureFormat::RGBABC3:
			return VK_FORMAT_BC3_UNORM_BLOCK;
		case ETextureFormat::RGBABC6:
			return VK_FORMAT_BC6H_UFLOAT_BLOCK;
		case ETextureFormat::RGBC5:
			return VK_FORMAT_BC5_UNORM_BLOCK;
		case ETextureFormat::RG32F:
			return VK_FORMAT_R32G32_SFLOAT;
		default:
			return VK_FORMAT_UNDEFINED;
		}
	}

	constexpr VkImageUsageFlags ConvertTextureUsage(ETextureUsage flags) {
		VkImageUsageFlags outFlags = 0;

		if (EnumHasAllFlags(flags, ETextureUsage::Sampled))     outFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
		if (EnumHasAllFlags(flags, ETextureUsage::Storage))     outFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		if (EnumHasAllFlags(flags, ETextureUsage::ColorTarget)) outFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (EnumHasAllFlags(flags, ETextureUsage::DepthTarget)) outFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		if (EnumHasAllFlags(flags, ETextureUsage::CopySrc))     outFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		if (EnumHasAllFlags(flags, ETextureUsage::CopyDst))     outFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		return outFlags;
	}

	constexpr VkImageLayout ConvertImageLayout(EGPUAccess flags) {

		if (EnumHasAnyFlags(flags, EGPUAccess::UAVCompute | EGPUAccess::UAVGraphics))      return VK_IMAGE_LAYOUT_GENERAL;
		else if (EnumHasAnyFlags(flags, EGPUAccess::SRVCompute | EGPUAccess::SRVGraphics)) return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::CopySrc))                              return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::CopyDst))                              return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::ColorTarget))                          return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::DepthTarget))                          return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		else                                                                               return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	constexpr VkAccessFlags2 ConvertResourceState(EGPUAccess flags) {
		VkAccessFlags2 outFlags = 0;

		if (EnumHasAnyFlags(flags, EGPUAccess::UAVCompute | EGPUAccess::UAVGraphics)) outFlags |= VK_ACCESS_2_SHADER_WRITE_BIT;
		if (EnumHasAnyFlags(flags, EGPUAccess::SRVCompute | EGPUAccess::SRVGraphics)) outFlags |= VK_ACCESS_2_SHADER_READ_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::CopySrc))                              outFlags |= VK_ACCESS_2_TRANSFER_READ_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::CopyDst))                              outFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::ColorTarget))                          outFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::DepthTarget))                          outFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::IndirectArgs))                         outFlags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;

		return outFlags;
	}

	constexpr VkPipelineStageFlags2 ConvertPipelineStage(EGPUAccess flags) {
		VkPipelineStageFlags2 outFlags = 0;

		if (EnumHasAnyFlags(flags, EGPUAccess::SRVCompute | EGPUAccess::UAVCompute))   outFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		if (EnumHasAnyFlags(flags, EGPUAccess::SRVGraphics | EGPUAccess::UAVGraphics)) outFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::IndirectArgs))                          outFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
		if (EnumHasAnyFlags(flags, EGPUAccess::CopySrc | EGPUAccess::CopyDst))         outFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::ColorTarget))                           outFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::DepthTarget))                           outFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

		return outFlags;
	}

	constexpr VkBufferUsageFlags ConvertBufferUsage(EBufferUsage flags) {
		VkBufferUsageFlags outFlags = 0;

		if (EnumHasAllFlags(flags, EBufferUsage::Constant)) outFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::Storage))  outFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::CopyDst))  outFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::CopySrc))  outFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::Indirect)) outFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::Index))    outFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		return outFlags;
	}

	constexpr VkFrontFace ConvertFrontFace(EFrontFace face) {
		switch (face)
		{
		case EFrontFace::ClockWise:
			return VK_FRONT_FACE_CLOCKWISE;
		case EFrontFace::CounterClockWise:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		default:
			return VK_FRONT_FACE_COUNTER_CLOCKWISE;
		}
	}

	constexpr VkImageSubresourceRange SubresourceRangeToVulkan(const SubresourceRange& range) {
		VkImageSubresourceRange outRange{};
		outRange.aspectMask = VK_IMAGE_ASPECT_NONE;
		outRange.baseArrayLayer = range.BaseLayer;
		outRange.baseMipLevel = range.BaseMip;
		outRange.layerCount = range.NumLayers;
		outRange.levelCount = range.NumMips;

		return outRange;
	}

	constexpr VkPrimitiveTopology PrimitiveTopologyToVulkan(EPrimitiveTopology topology);
	constexpr VkCullModeFlags CullModeToVulkan(ECullMode mode);
	constexpr VkBlendFactor BlendFactorToVulkan(EBlendFactor factor);
	constexpr VkBlendOp BlendOpToVulkan(EBlendOp op);
	constexpr VkCompareOp ComparisonFuncToVulkan(EComparisonFunc func);
	constexpr VkStencilOp StencilOpToVulkan(EStencilOp op);

	VulkanCommandList::VulkanCommandList(ERHIQueue queue, VulkanRHIDevice& device)
		: m_Device(device), m_SubmitID(0)
	{
		VkCommandPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = m_Device.GetQueue(queue).GetFamilyIndex();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK(vkCreateCommandPool(m_Device.GetDeviceHandle(), &poolInfo, nullptr, &m_Pool));

		VkCommandBufferAllocateInfo cmdInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		cmdInfo.commandPool = m_Pool;
		cmdInfo.commandBufferCount = 1;
		cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(m_Device.GetDeviceHandle(), &cmdInfo, &m_CmdBuffer));
	}

	VulkanCommandList::~VulkanCommandList() {
		vkDestroyCommandPool(m_Device.GetDeviceHandle(), m_Pool, nullptr);
	}

	void VulkanCommandList::MipMapTexture2D(IRHITexture2D* tex, EGPUAccess lastAccess, EGPUAccess newAccess, uint32 numMips) {}

	void VulkanCommandList::CopyTexture(IRHITexture* src, const TextureCopyRegion& srcRegion, IRHITexture* dst, const TextureCopyRegion& dstRegion, Vec2Uint copySize) {
		VkImage vkSrc = (VkImage)src->GetNative();
		VkImage vkDst = (VkImage)dst->GetNative();

		VkImageCopy2 copyRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_COPY_2 };
		copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.srcSubresource.baseArrayLayer = srcRegion.BaseArrayLayer;
		copyRegion.srcSubresource.mipLevel = srcRegion.MipLevel;
		copyRegion.srcSubresource.layerCount = srcRegion.LayerCount;
		copyRegion.srcOffset = { srcRegion.Offset.x, srcRegion.Offset.y, srcRegion.Offset.z };

		copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.dstSubresource.baseArrayLayer = dstRegion.BaseArrayLayer;
		copyRegion.dstSubresource.mipLevel = dstRegion.MipLevel;
		copyRegion.dstSubresource.layerCount = dstRegion.LayerCount;
		copyRegion.dstOffset = { dstRegion.Offset.x, dstRegion.Offset.y, dstRegion.Offset.z };

		copyRegion.extent.width = copySize.x;
		copyRegion.extent.height = copySize.y;
		copyRegion.extent.depth = 1;

		VkCopyImageInfo2 copyInfo{};
		copyInfo.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2;
		copyInfo.srcImage = vkSrc;
		copyInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		copyInfo.dstImage = vkDst;
		copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		copyInfo.regionCount = 1;
		copyInfo.pRegions = &copyRegion;

		vkCmdCopyImage2(m_CmdBuffer, &copyInfo);
	}

	void VulkanCommandList::ClearTexture(IRHITexture* tex, const SubresourceRange& range, EGPUAccess access, const Vec4& color) {
		VkImage vkTex = (VkImage)tex->GetNative();

		VkClearColorValue clearValue = { color.x, color.y, color.z, color.w };
		VkImageSubresourceRange clearRange = VulkanUtils::SubresourceRangeToVulkan(range);
		clearRange.aspectMask = VulkanUtils::TextureFormatToAspect(tex->GetFormat());

		vkCmdClearColorImage(m_CmdBuffer, vkTex, VulkanUtils::GPUAccessToVulkanLayout(access), &clearValue, 1, &clearRange);
	}

	void VulkanCommandList::CopyFromTextureToCPU(IRHITexture* src, const SubresourceCopyRegion& region, IRHIBuffer* dst) {
		if (IsTextureCompressed(src->GetFormat())) {
			ENGINE_ERROR("Cannot copy from texture to buffer if texture is compressed!");
			return;
		}

		VkImage vkSrc = (VkImage)src->GetNative();
		VkBuffer vkDst = (VkBuffer)dst->GetNative();

		VkBufferImageCopy2 vkRegion{.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2};
		vkRegion.bufferOffset = region.DataOffset;
		vkRegion.bufferRowLength = 0;
		vkRegion.bufferImageHeight = 0;

		vkRegion.imageSubresource.aspectMask = VulkanUtils::TextureFormatToAspect(src->GetFormat());
		vkRegion.imageSubresource.mipLevel = region.MipLevel;
		vkRegion.imageSubresource.baseArrayLayer = region.BaseLayer;
		vkRegion.imageSubresource.layerCount = region.NumLayers;

		Vec2Uint mipExtent = TextureMipExtents(src->GetSizeXYZ().x, src->GetSizeXYZ().y, region.MipLevel);
		vkRegion.imageExtent = { mipExtent.x, mipExtent.y, 1 };
		vkRegion.imageOffset = { 0, 0, 0 };

		VkCopyImageToBufferInfo2 info{ .sType = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2};
		info.srcImage = vkSrc;
		info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		info.dstBuffer = vkDst;
		info.regionCount = 1;
		info.pRegions = &vkRegion;

		vkCmdCopyImageToBuffer2(m_CmdBuffer, &info);
	}

	void VulkanCommandList::BarrierTexture(IRHITexture* texture, const TextureBarrierRegion* regions, uint32 numRegions) {
		VkImage vkTex = (VkImage)texture->GetNative();

		std::vector<VkImageMemoryBarrier2> barriers{};
		barriers.reserve(numRegions);

		for (uint32 i = 0; i < numRegions; i++) {
			const TextureBarrierRegion& region = regions[i];

			VkImageMemoryBarrier2 b{ .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };
			b.srcStageMask = VulkanUtils::GPUAccessToVulkanStage(region.LastAccess);
			b.srcAccessMask = VulkanUtils::GPUAccessToVulkanAccess(region.LastAccess);
			b.oldLayout = VulkanUtils::GPUAccessToVulkanLayout(region.LastAccess);
			b.dstStageMask = VulkanUtils::GPUAccessToVulkanStage(region.NewAccess);
			b.dstAccessMask = VulkanUtils::GPUAccessToVulkanAccess(region.NewAccess);
			b.newLayout = VulkanUtils::GPUAccessToVulkanLayout(region.NewAccess);
			b.subresourceRange = VulkanUtils::SubresourceRangeToVulkan(region.Range);
			b.subresourceRange.aspectMask = VulkanUtils::TextureFormatToAspect(texture->GetFormat());

			barriers.push_back(b);
		}

		VkDependencyInfo depInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		depInfo.imageMemoryBarrierCount = numRegions;
		depInfo.pImageMemoryBarriers = barriers.data();

		vkCmdPipelineBarrier2(m_CmdBuffer, &depInfo);
	}

	void VulkanCommandList::CopyBuffer(IRHIBuffer* srcBuffer, IRHIBuffer* dstBuffer, uint64 srcOffset, uint64 dstOffset, uint64 size) {
		VkBuffer vkSrc = (VkBuffer)srcBuffer->GetNative();
		VkBuffer vkDst = (VkBuffer)dstBuffer->GetNative();

		VkBufferCopy2 copy{ .sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2 };
		copy.dstOffset = dstOffset;
		copy.srcOffset = srcOffset;
		copy.size = size;

		VkCopyBufferInfo2 info{ .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2 };
		info.srcBuffer = vkSrc;
		info.dstBuffer = vkDst;
		info.regionCount = 1;
		info.pRegions = &copy;

		vkCmdCopyBuffer2(m_CmdBuffer, &info);
	}

	void VulkanCommandList::BarrierBuffer(IRHIBuffer* buffer, uint64 size, uint64 offset, EGPUAccess lastAccess, EGPUAccess newAccess) {
		VkBuffer vkBuff = (VkBuffer)buffer->GetNative();

		VkBufferMemoryBarrier2 barrier{ .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };
		barrier.srcStageMask = VulkanUtils::GPUAccessToVulkanStage(lastAccess);
		barrier.srcAccessMask = VulkanUtils::GPUAccessToVulkanAccess(lastAccess);
		barrier.dstStageMask = VulkanUtils::GPUAccessToVulkanStage(newAccess);
		barrier.dstAccessMask = VulkanUtils::GPUAccessToVulkanAccess(newAccess);
		barrier.buffer = vkBuff;
		barrier.offset = offset;
		barrier.size = size;

		VkDependencyInfo depInfo{ .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		depInfo.bufferMemoryBarrierCount = 1;
		depInfo.pBufferMemoryBarriers = &barrier;

		vkCmdPipelineBarrier2(m_CmdBuffer, &depInfo);
	}

	void VulkanCommandList::FillBuffer(IRHIBuffer* buffer, uint64 size, uint64 offset, uint32 value) {
		VkBuffer vkBuff = (VkBuffer)buffer->GetNative();
		vkCmdFillBuffer(m_CmdBuffer, vkBuff, offset, size, value);
	}

	VulkanBuffer::VulkanBuffer(uint64 size, EBufferUsage usage, VulkanRHIDevice& device)
		: IRHIBuffer(size, usage), m_Device(device), m_MappedData(nullptr)
	{
		VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = size;
		bufferInfo.usage = VulkanUtils::BufferUsageToVulkan(usage);

		VmaAllocationCreateInfo vmaAllocInfo{};
		vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		//vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VK_CHECK(vmaCreateBuffer(m_Device.GetAllocatorHandle(), &bufferInfo, &vmaAllocInfo, &m_Buffer,
			&m_Allocation, nullptr));

		if (EnumHasAnyFlags(usage, EBufferUsage::Mapped)) {
			vmaMapMemory(m_Device.GetAllocatorHandle(), m_Allocation, &m_MappedData);
		}
	}

	VulkanBuffer::~VulkanBuffer() {
		m_Device.DestroyResource({ 
			.Buffer = m_Buffer, 
			.Allocation = m_Allocation
			});
	}

	VulkanTexture2D::VulkanTexture2D(uint32 width, uint32 height, uint32 numMips, ETextureFormat format, ETextureUsage usage, VulkanRHIDevice& device)
		: IRHITexture2D(width, height, numMips, format, usage), m_Device(device)
	{
		VkImageCreateInfo imgInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imgInfo.imageType = VK_IMAGE_TYPE_2D;
		imgInfo.format = VulkanUtils::TextureFormatToVulkan(format);
		imgInfo.extent = VkExtent3D{ width, height, 1 };
		imgInfo.mipLevels = numMips;
		imgInfo.arrayLayers = 1;
		imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.usage = VulkanUtils::TextureUsageToVulkan(usage);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK(vmaCreateImage(m_Device.GetAllocatorHandle(), &imgInfo, &allocInfo, &m_Image, &m_Allocation, nullptr));
	}

	VulkanTexture2D::~VulkanTexture2D() {
		m_Device.DestroyResource({
			.Image = m_Image,
			.Allocation = m_Allocation
			});
	}

	VulkanTextureCube::VulkanTextureCube(uint32 size, uint32 numMips, ETextureFormat format, ETextureUsage usage, VulkanRHIDevice& device)
		: IRHITextureCube(size, numMips, format, usage), m_Device(device)
	{
		VkImageCreateInfo imgInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imgInfo.imageType = VK_IMAGE_TYPE_2D;
		imgInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		imgInfo.format = VulkanUtils::TextureFormatToVulkan(format);
		imgInfo.extent = VkExtent3D{ size, size, 1 };
		imgInfo.mipLevels = numMips;
		imgInfo.arrayLayers = 6;
		imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.usage = VulkanUtils::TextureUsageToVulkan(usage);

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VK_CHECK(vmaCreateImage(m_Device.GetAllocatorHandle(), &imgInfo, &allocInfo, &m_Image, &m_Allocation, nullptr));
	}

	VulkanTextureCube::~VulkanTextureCube() {
		m_Device.DestroyResource({
			.Image = m_Image,
			.Allocation = m_Allocation
			});
	}

	VulkanTextureView::VulkanTextureView(const SubresourceRange& range, IRHITexture* tex, VulkanRHIDevice& device)
		: IRHITextureView(range), m_Device(device)
	{
		VkImageViewCreateInfo viewInfo{.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };

		// FIXME
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.image = (VkImage)tex->GetNative();
		viewInfo.format = VulkanUtils::TextureFormatToVulkan(tex->GetFormat());
		viewInfo.subresourceRange = VulkanUtils::SubresourceRangeToVulkan(range);
		viewInfo.subresourceRange.aspectMask = VulkanUtils::TextureFormatToAspect(tex->GetFormat());

		VK_CHECK(vkCreateImageView(m_Device.GetDeviceHandle(), &viewInfo, nullptr, &m_View));
	}

	VulkanTextureView::~VulkanTextureView() {
		m_Device.DestroyResource({
			.View = m_View
			});
	}

	VulkanSamplerState::VulkanSamplerState(const SamplerStateDesc& desc, VulkanRHIDevice& device) 
		: m_Device(device) 
	{
		VkSamplerCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		info.magFilter = VulkanUtils::SamplerFilterToVulkan(desc.Filter);
		info.minFilter = info.magFilter;
		info.mipmapMode = VulkanUtils::SamplerMipMapModeToVulkan(desc.Filter);
		info.addressModeU = VulkanUtils::SamplerAddressToVulkan(desc.AddressU);
		info.addressModeV = VulkanUtils::SamplerAddressToVulkan(desc.AddressV);
		info.addressModeW = VulkanUtils::SamplerAddressToVulkan(desc.AddressW);
		info.minLod = desc.MinLOD;
		info.maxLod = desc.MaxLOD;
		info.anisotropyEnable = false;
		info.maxAnisotropy = (float)desc.MaxAnisotropy;

		VkSamplerReductionModeCreateInfoEXT createInfoReduction{};
		if (desc.Reduction != ESamplerReduction::Standard) {
			createInfoReduction.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
			createInfoReduction.reductionMode = VulkanUtils::SamplerReductionToVulkan(desc.Reduction);
			info.pNext = &createInfoReduction;
		}

		VK_CHECK(vkCreateSampler(m_Device.GetDeviceHandle(), &info, nullptr, &m_Sampler));
	}

	VulkanSamplerState::~VulkanSamplerState() {
		vkDestroySampler(m_Device.GetDeviceHandle(), m_Sampler, nullptr);
	}

	VulkanShader::VulkanShader(const std::span<uint8>& bytecode, VkShaderStageFlags stage, VulkanRHIDevice& device)
		: m_Device(device) 
	{
		VkShaderModuleCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		info.codeSize = bytecode.size();
		info.pCode = (uint32*)bytecode.data();

		VK_CHECK(vkCreateShaderModule(m_Device.GetDeviceHandle(), &info, nullptr, &m_Module));

		// reflection
		{
			SpvReflectShaderModule module;
			SpvReflectResult result = spvReflectCreateShaderModule(bytecode.size(), bytecode.data(), &module);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			uint32 bindingCount = 0;
			result = spvReflectEnumerateDescriptorBindings(&module, &bindingCount, nullptr);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
			result = spvReflectEnumerateDescriptorBindings(&module, &bindingCount, bindings.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			uint32 pushCount = 0;
			result = spvReflectEnumeratePushConstantBlocks(&module, &pushCount, nullptr);
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			std::vector<SpvReflectBlockVariable*> pushConstants(pushCount);
			result = spvReflectEnumeratePushConstantBlocks(&module, &pushCount, pushConstants.data());
			assert(result == SPV_REFLECT_RESULT_SUCCESS);

			for (auto x : pushConstants) {
				m_PushConstants.stageFlags = stage;
				m_PushConstants.size = x->size;
				m_PushConstants.offset = x->offset;
			}

			for (auto x : bindings) {
				auto& b = m_Bindings.emplace_back();

				b.stageFlags = stage;
				b.binding = x->binding;
				b.descriptorCount = x->count;
				b.descriptorType = (VkDescriptorType)x->descriptor_type;

				if (x->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER
					&& x->binding >= VK_BINDING_SHIFT_S - VK_IMMUTABLE_SAMPLER_FIRST_SLOT) 
				{
					b.pImmutableSamplers = m_Device.GetImmutableSamplers() + (x->binding - VK_BINDING_SHIFT_S - VK_IMMUTABLE_SAMPLER_FIRST_SLOT);
				}
			}

			spvReflectDestroyShaderModule(&module);
		}
	}

	VulkanShader::~VulkanShader() {
		vkDestroyShaderModule(m_Device.GetDeviceHandle(), m_Module, nullptr);
	}

	VulkanPipelineState::VulkanPipelineState(const PipelineStateDesc& desc, VulkanRHIDevice& device) 
		: m_Device(device), m_PushConstants{}
	{
		// based of wicked engine pso creation
		{
			auto insertShader = [&, this](IRHIShader* shader) {
				if (!shader)
					return;

				VulkanShader* vkShader = (VulkanShader*)vkShader;
				auto& shaderBindings = vkShader->GetBindings();

				for (auto& x : shaderBindings) {
					bool found = false;

					for (auto& y : m_LayoutBindings) {
						if (x.binding == y.binding) {
							// check for overlapping descriptors
							assert(x.descriptorCount == y.descriptorCount
								&& x.descriptorType == y.descriptorType);

							y.stageFlags |= x.stageFlags;
							found = true;
							break;
						}
					}

					if (!found) {
						m_LayoutBindings.push_back(x);
					}
				}

				auto pushConstants = vkShader->GetPushConstants();
				if (pushConstants.size > 0) {
					m_PushConstants.stageFlags |= pushConstants.stageFlags;
					m_PushConstants.size = std::max(m_PushConstants.size, pushConstants.size);
					m_PushConstants.offset = std::min(m_PushConstants.offset, pushConstants.offset);
				}
				};

			insertShader(desc.ComputeShader);
			insertShader(desc.VertexShader);
			insertShader(desc.PixelShader);
		}
		{
			VkDescriptorSetLayoutCreateInfo setLayoutInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			setLayoutInfo.bindingCount = (uint32)m_LayoutBindings.size();
			setLayoutInfo.pBindings = m_LayoutBindings.data();

			VK_CHECK(vkCreateDescriptorSetLayout(m_Device.GetDeviceHandle(), &setLayoutInfo, nullptr, &m_SetLayout));
		}
		{
			VulkanPSOLayoutHash layoutHash{};
			for (auto& x : m_LayoutBindings) {
				layoutHash.Bindings.push_back(x);
			}

			layoutHash.PushConstants = m_PushConstants;
			layoutHash.ComputeHash();

			VulkanPSOLayout cachedLayout = m_Device.CreateCachedPSOLayout(layoutHash);
			m_Layout = cachedLayout.Layout;
			m_SetLayout = cachedLayout.SetLayout;
		}

		if (desc.ComputeShader) {

			VkPipelineShaderStageCreateInfo stageInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			stageInfo.module = (VkShaderModule)desc.ComputeShader->GetNative();
			stageInfo.pName = "CSMain";

			VkComputePipelineCreateInfo info{ .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
			info.stage = stageInfo;
			info.layout = m_Layout;

			VK_CHECK(vkCreateComputePipelines(m_Device.GetDeviceHandle(), m_Device.GetPipelineCacheHandle(),
				1, &info, nullptr, &m_Pipeline));
		}
		else {
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
			inputInfo.topology = VulkanUtils::PrimitiveTopologyToVulkan(desc.PrimitiveTopology);
			inputInfo.primitiveRestartEnable = false;

			VkPipelineRasterizationStateCreateInfo rasterInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterInfo.polygonMode = desc.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
			rasterInfo.lineWidth = 1.f;
			rasterInfo.cullMode = VulkanUtils::CullModeToVulkan(desc.CullMode);
			rasterInfo.frontFace = VulkanUtils::FrontFaceToVulkan(desc.FrontFace);

			VkFormat                            rtFormats[8] = {};
			VkPipelineColorBlendAttachmentState blendStates[8] = {};

			for (uint8 i = 0; i < desc.NumRenderTargets; i++) {
				auto& rt = desc.RenderTargets[i];

				blendStates[i].blendEnable = rt.EnableBlend;
				blendStates[i].srcColorBlendFactor = VulkanUtils::BlendFactorToVulkan(rt.SrcBlend);
				blendStates[i].dstColorBlendFactor = VulkanUtils::BlendFactorToVulkan(rt.DstBlend);
				blendStates[i].colorBlendOp = VulkanUtils::BlendOpToVulkan(rt.BlendOp);
				blendStates[i].srcAlphaBlendFactor = VulkanUtils::BlendFactorToVulkan(rt.SrcBlendAlpha);
				blendStates[i].dstAlphaBlendFactor = VulkanUtils::BlendFactorToVulkan(rt.DstBlendAlpha);
				blendStates[i].alphaBlendOp = VulkanUtils::BlendOpToVulkan(rt.BlendOpAlpha);
				blendStates[i].colorWriteMask = (uint32)rt.ColorMask;

				rtFormats[i] = VulkanUtils::TextureFormatToVulkan(rt.Format);
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
			renderingInfo.depthAttachmentFormat = VulkanUtils::TextureFormatToVulkan(desc.DepthFormat);

			// TODO: add stencil support
			renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			depthStencilInfo.depthTestEnable = desc.DepthEnable;
			depthStencilInfo.depthWriteEnable = desc.DepthWriteEnable;
			depthStencilInfo.depthCompareOp = VulkanUtils::ComparisonFuncToVulkan(desc.DepthFunc);
			depthStencilInfo.depthBoundsTestEnable = desc.DepthClipEnable;
			depthStencilInfo.stencilTestEnable = desc.StencilEnable;
			
			if (desc.StencilEnable) {
				depthStencilInfo.front.failOp = VulkanUtils::StencilOpToVulkan(desc.FrontStencil.FailOp);
				depthStencilInfo.front.passOp = VulkanUtils::StencilOpToVulkan(desc.FrontStencil.PassOp);
				depthStencilInfo.front.depthFailOp = VulkanUtils::StencilOpToVulkan(desc.FrontStencil.DepthFailOp);
				depthStencilInfo.front.compareOp = VulkanUtils::ComparisonFuncToVulkan(desc.FrontStencil.Func);
				depthStencilInfo.front.compareMask = desc.FrontStencil.ReadMask;
				depthStencilInfo.front.writeMask = desc.FrontStencil.WriteMask;
				depthStencilInfo.front.reference = 0;

				depthStencilInfo.back.failOp = VulkanUtils::StencilOpToVulkan(desc.BackStencil.FailOp);
				depthStencilInfo.back.passOp = VulkanUtils::StencilOpToVulkan(desc.BackStencil.PassOp);
				depthStencilInfo.back.depthFailOp = VulkanUtils::StencilOpToVulkan(desc.BackStencil.DepthFailOp);
				depthStencilInfo.back.compareOp = VulkanUtils::ComparisonFuncToVulkan(desc.BackStencil.Func);
				depthStencilInfo.back.compareMask = desc.BackStencil.ReadMask;
				depthStencilInfo.back.writeMask = desc.BackStencil.WriteMask;
				depthStencilInfo.back.reference = 0;
			}
			else {
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

			VK_CHECK(vkCreateGraphicsPipelines(m_Device.GetDeviceHandle(), m_Device.GetPipelineCacheHandle(), 1, &pipelineInfo, nullptr, &m_Pipeline));
		}
	}

	VulkanPipelineState::~VulkanPipelineState() {}
}