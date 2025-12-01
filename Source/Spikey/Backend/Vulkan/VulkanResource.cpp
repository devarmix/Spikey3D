#include <Backend/Vulkan/VulkanRHI.h>
#include <Backend/Vulkan/VulkanResource.h>
#include <Backend/Vulkan/VulkanUtils.h>

namespace Spikey {

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
		: IRHIBuffer(size, usage), m_Device(device), m_SRVHandle(-1), m_UAVHandle(-1), m_MappedData(nullptr)
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
			.Allocation = m_Allocation, 
			.SRVHandle = m_SRVHandle, 
			.UAVHandle = m_UAVHandle 
			});
	}

	int32 VulkanBuffer::GetSRVHandle() {
		if (m_SRVHandle == -1) {
			auto& heap = m_Device.GetBufferSRVHeap();
			std::scoped_lock lock(heap.Mutex);

			m_SRVHandle = heap.Allocate();

			VkDescriptorBufferInfo info{};
			info.buffer = m_Buffer;
			info.offset = 0;
			info.range = GetSize();

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstBinding = 0;
			write.dstArrayElement = m_SRVHandle;
			write.dstSet = heap.Set;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.pBufferInfo = &info;

			vkUpdateDescriptorSets(m_Device.GetDeviceHandle(), 1, &write, 0, nullptr);
		}

		return m_SRVHandle;
	}

	int32 VulkanBuffer::GetUAVHandle() {
		if (m_UAVHandle == -1) {
			auto& heap = m_Device.GetBufferUAVHeap();
			std::scoped_lock lock(heap.Mutex);

			m_UAVHandle = heap.Allocate();

			VkDescriptorBufferInfo info{};
			info.buffer = m_Buffer;
			info.offset = 0;
			info.range = GetSize();

			VkWriteDescriptorSet write{};
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.dstBinding = 0;
			write.dstArrayElement = m_UAVHandle;
			write.dstSet = heap.Set;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			write.pBufferInfo = &info;

			vkUpdateDescriptorSets(m_Device.GetDeviceHandle(), 1, &write, 0, nullptr);
		}

		return m_UAVHandle;
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
		: IRHITextureView(range), m_Device(device), m_SRVHandle(-1), m_UAVHandle(-1) 
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
			.View = m_View,
			.SRVHandle = m_SRVHandle,
			.UAVHandle = m_UAVHandle
			});
	}

	int32 VulkanTextureView::GetSRVHandle() {
		if (m_SRVHandle == -1) {
			auto& heap = m_Device.GetTextureSRVHeap();
			std::scoped_lock lock(heap.Mutex);

			m_SRVHandle = heap.Allocate();

			VkDescriptorImageInfo info{};
			info.sampler = nullptr;
			info.imageView = m_View;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkWriteDescriptorSet write{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			write.dstBinding = 0;
			write.dstArrayElement = m_SRVHandle;
			write.dstSet = heap.Set;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			write.pImageInfo = &info;

			vkUpdateDescriptorSets(m_Device.GetDeviceHandle(), 1, &write, 0, nullptr);
		}

		return m_SRVHandle;
	}

	int32 VulkanTextureView::GetUAVHandle() {
		if (m_UAVHandle == -1) {
			auto& heap = m_Device.GetTextureUAVHeap();
			std::scoped_lock lock(heap.Mutex);

			m_UAVHandle = heap.Allocate();

			VkDescriptorImageInfo info{};
			info.sampler = nullptr;
			info.imageView = m_View;
			info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			VkWriteDescriptorSet write{ .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			write.dstBinding = 0;
			write.dstArrayElement = m_UAVHandle;
			write.dstSet = heap.Set;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			write.pImageInfo = &info;

			vkUpdateDescriptorSets(m_Device.GetDeviceHandle(), 1, &write, 0, nullptr);
		}

		return m_UAVHandle;
	}

	VulkanShader::VulkanShader(const std::span<uint8>& bytecode, VulkanRHIDevice& device)
		: m_Device(device) 
	{
		VkShaderModuleCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		info.codeSize = bytecode.size();
		info.pCode = (uint32*)bytecode.data();

		VK_CHECK(vkCreateShaderModule(m_Device.GetDeviceHandle(), &info, nullptr, &m_Module));
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
				auto  shaderPushConst = vkShader->GetPushConstants();

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

				if (shaderPushConst.size > 0) {
					m_PushConstants.size = std::max(m_PushConstants.size, shaderPushConst.size);
					m_PushConstants.offset = std::min(m_PushConstants.offset, shaderPushConst.offset);
					m_PushConstants.stageFlags |= shaderPushConst.stageFlags;
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

		}
	}

	VulkanPipelineState::~VulkanPipelineState() {}
}