#include <Backend/Vulkan/VulkanBackend.h>
#include <spirv_reflect.h>

namespace Spikey {

	constexpr VkFormat ConvertVkImageFormat(ETextureFormat format) 
	{
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

	constexpr VkImageLayout ConvertVkImageLayout(EGPUAccess flags) 
	{
		if (EnumHasAnyFlags(flags, EGPUAccess::UAVCompute | EGPUAccess::UAVGraphics))      return VK_IMAGE_LAYOUT_GENERAL;
		else if (EnumHasAnyFlags(flags, EGPUAccess::SRVCompute | EGPUAccess::SRVGraphics)) return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::CopySrc))                              return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::CopyDst))                              return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::ColorTarget))                          return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else if (EnumHasAllFlags(flags, EGPUAccess::DepthTarget))                          return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		else                                                                               return VK_IMAGE_LAYOUT_UNDEFINED;
	}

	constexpr VkAccessFlags2 ConvertVkResourceState(EGPUAccess flags) 
	{
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

	constexpr VkPipelineStageFlags2 ConvertVkPipelineStage(EGPUAccess flags) 
	{
		VkPipelineStageFlags2 outFlags = 0;

		if (EnumHasAnyFlags(flags, EGPUAccess::SRVCompute | EGPUAccess::UAVCompute))   outFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
		if (EnumHasAnyFlags(flags, EGPUAccess::SRVGraphics | EGPUAccess::UAVGraphics)) outFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::IndirectArgs))                          outFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
		if (EnumHasAnyFlags(flags, EGPUAccess::CopySrc | EGPUAccess::CopyDst))         outFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::ColorTarget))                           outFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
		if (EnumHasAllFlags(flags, EGPUAccess::DepthTarget))                           outFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;

		return outFlags;
	}

	constexpr VkBufferUsageFlags ConvertVkBufferUsage(EBufferUsage flags) 
	{
		VkBufferUsageFlags outFlags = 0;

		if (EnumHasAllFlags(flags, EBufferUsage::Constant)) outFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::Storage))  outFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::CopyDst))  outFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::CopySrc))  outFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::Indirect)) outFlags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		if (EnumHasAllFlags(flags, EBufferUsage::Index))    outFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		return outFlags;
	}

	constexpr VkFrontFace ConvertVkFrontFace(EFrontFace face) 
	{
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

	/*
	constexpr VkImageSubresourceRange SubresourceRangeToVulkan(const SubresourceRange& range) {
		VkImageSubresourceRange outRange{};
		outRange.aspectMask = VK_IMAGE_ASPECT_NONE;
		outRange.baseArrayLayer = range.BaseLayer;
		outRange.baseMipLevel = range.BaseMip;
		outRange.layerCount = range.NumLayers;
		outRange.levelCount = range.NumMips;

		return outRange;
	} */

	constexpr VkPrimitiveTopology ConvertVkPrimitiveTopology(EPrimitiveTopology topology) 
	{
		switch (topology)
		{
		case EPrimitiveTopology::PointList:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case EPrimitiveTopology::LineList:
			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case EPrimitiveTopology::LineStrip:
			return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case EPrimitiveTopology::TriangleList:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case EPrimitiveTopology::TriangleStrip:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case EPrimitiveTopology::TriangleFan:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case EPrimitiveTopology::TriangleListWithAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case EPrimitiveTopology::TriangleStripWithAdjacency:
			return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case EPrimitiveTopology::PatchList:
			return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		default:
			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		}
	}

	constexpr VkCullModeFlags ConvertVkCullMode(ECullMode mode) 
	{
		switch (mode)
		{
		case ECullMode::FrontFace:
			return VK_CULL_MODE_FRONT_BIT;
		case ECullMode::BackFace:
			return VK_CULL_MODE_BACK_BIT;
		default:
			return VK_CULL_MODE_NONE;
		}
	}

	constexpr VkBlendFactor ConvertVkBlendFactor(EBlendFactor factor) 
	{
		switch (factor)
		{
		case EBlendFactor::Zero:
			return VK_BLEND_FACTOR_ZERO;
		case EBlendFactor::One:
			return VK_BLEND_FACTOR_ONE;
		case EBlendFactor::SrcColor:
			return VK_BLEND_FACTOR_SRC_COLOR;
		case EBlendFactor::OneMinusSrcColor:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		case EBlendFactor::SrcAlpha:
			return VK_BLEND_FACTOR_SRC_ALPHA;
		case EBlendFactor::OneMinusSrcAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		case EBlendFactor::DstAlpha:
			return VK_BLEND_FACTOR_DST_ALPHA;
		case EBlendFactor::OneMinusDstAlpha:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		case EBlendFactor::DstColor:
			return VK_BLEND_FACTOR_DST_COLOR;
		case EBlendFactor::OneMinusDstColor:
			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		case EBlendFactor::SrcAlphaSaturate:
			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		case EBlendFactor::ConstantColor:
			return VK_BLEND_FACTOR_CONSTANT_COLOR;
		case EBlendFactor::OneMinusConstantColor:
			return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		case EBlendFactor::Src1Color:
			return VK_BLEND_FACTOR_SRC1_COLOR;
		case EBlendFactor::OneMinusSrc1Color:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		case EBlendFactor::Src1Alpha:
			return VK_BLEND_FACTOR_SRC1_ALPHA;
		case EBlendFactor::OneMinusSrc1Alpha:
			return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		default:
			return VK_BLEND_FACTOR_ZERO;
		}
	}

	constexpr VkBlendOp ConvertVkBlendOp(EBlendOp op) 
	{
		switch (op)
		{
		case EBlendOp::Add:
			return VK_BLEND_OP_ADD;
		case EBlendOp::Subtract:
			return VK_BLEND_OP_SUBTRACT;
		case EBlendOp::ReverseSubtract:
			return VK_BLEND_OP_REVERSE_SUBTRACT;
		case EBlendOp::Min:
			return VK_BLEND_OP_MIN;
		case EBlendOp::Max:
			return VK_BLEND_OP_MAX;
		default:
			return VK_BLEND_OP_ADD;
		}
	}

	constexpr VkCompareOp ConvertVkCompareOp(EComparisonFunc func) 
	{
		switch (func)
		{
		case EComparisonFunc::Less:
			return VK_COMPARE_OP_LESS;
		case EComparisonFunc::Equal:
			return VK_COMPARE_OP_EQUAL;
		case EComparisonFunc::LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case EComparisonFunc::Greater:
			return VK_COMPARE_OP_GREATER;
		case EComparisonFunc::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case EComparisonFunc::GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case EComparisonFunc::Always:
			return VK_COMPARE_OP_ALWAYS;
		default:
			return VK_COMPARE_OP_NEVER;
		}
	}

	constexpr VkStencilOp ConvertVkStencilOp(EStencilOp op) 
	{
		switch (op)
		{
		case EStencilOp::Keep:
			return VK_STENCIL_OP_KEEP;
		case EStencilOp::Zero:
			return VK_STENCIL_OP_ZERO;
		case EStencilOp::Replace:
			return VK_STENCIL_OP_REPLACE;
		case EStencilOp::IncrementAndClamp:
			return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
		case EStencilOp::DecrementAndClamp:
			return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
		case EStencilOp::Invert:
			return VK_STENCIL_OP_INVERT;
		case EStencilOp::IncrementAndWrap:
			return VK_STENCIL_OP_INCREMENT_AND_WRAP;
		case EStencilOp::DecrementAndWrap:
			return VK_STENCIL_OP_DECREMENT_AND_WRAP;
		default:
			return VK_STENCIL_OP_KEEP;
		}
	}

	constexpr VkSamplerAddressMode ConvertVkAddressMode(ESamplerAddress mode) 
	{
		switch (mode)
		{
		case ESamplerAddress::Clamp:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case ESamplerAddress::Wrap:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case ESamplerAddress::Mirror:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		default:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}

	constexpr VkImageAspectFlags ConvertVkImageAspect(ETextureFormat format) 
	{
		switch (format)
		{
		case ETextureFormat::D32F:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	constexpr VkSamplerReductionMode ConvertVkReductionMode(ESamplerReduction mode)
	{
		switch (mode)
		{
		case ESamplerReduction::Minimum:
			return VK_SAMPLER_REDUCTION_MODE_MIN;
		case ESamplerReduction::Maximum:
			return VK_SAMPLER_REDUCTION_MODE_MAX;
		default:
			return VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE;
		}
	}

	VulkanCommandList::VulkanCommandList(VulkanQueue* queue, VulkanDevice& device) 
		: m_Device(device)
	{
		VkCommandPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = queue->GetFamilyIndex();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

		VK_CHECK(vkCreateCommandPool(m_Device.GetDeviceHandle(), &poolInfo, nullptr, &m_Pool));

		VkCommandBufferAllocateInfo cmdInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdInfo.commandPool = m_Pool;
		cmdInfo.commandBufferCount = 1;

		VK_CHECK(vkAllocateCommandBuffers(m_Device.GetDeviceHandle(), &cmdInfo, &m_CmdBuffer));
	}

	VulkanCommandList::~VulkanCommandList() 
	{
		vkDestroyCommandPool(m_Device.GetDeviceHandle(), m_Pool, nullptr);
	}

	VulkanQueue::VulkanQueue(ERHIQueue queueID, VkQueue queue, uint32 familyIndex, VulkanDevice& device)
		: m_QueueID(queueID)
		, m_Queue(queue)
		, m_FamilyIndex(familyIndex)
		, m_Device(device)
	{
		VkSemaphoreTypeCreateInfo semaphoreTypeinfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO};
		semaphoreTypeinfo.initialValue = 0;
		semaphoreTypeinfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

		VkSemaphoreCreateInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		semaphoreInfo.pNext = &semaphoreTypeinfo;
		semaphoreInfo.flags = 0;

		VK_CHECK(vkCreateSemaphore(m_Device.GetDeviceHandle(), &semaphoreInfo, nullptr, &m_TrackingSemaphore));
	}

	VulkanQueue::~VulkanQueue() 
	{
		vkDestroySemaphore(m_Device.GetDeviceHandle(), m_TrackingSemaphore, nullptr);
	}

	TRefCountPtr<VulkanCommandList> VulkanQueue::CreateCommandList()
	{
		std::lock_guard lock(m_Mutex);

		TRefCountPtr<VulkanCommandList> list = nullptr;
		if (m_ListsPool.empty())
		{
			list = CreateRefCounted<VulkanCommandList>(this, m_Device);
		}
		else
		{
			list = m_ListsPool.back();
			m_ListsPool.pop_back();
		}

		return list;
	}

	void VulkanQueue::AddWaitSemaphore(VkSemaphore wait, uint64 value)
	{
		if (!wait)
			return;

		m_WaitSemaphores.push_back(wait);
		m_WaitSemaphoreValues.push_back(value);
	}

	void VulkanQueue::AddSignalSemaphore(VkSemaphore signal, uint64 value)
	{
		if (!signal)
			return;

		m_SignalSemaphores.push_back(signal);
		m_SignalSemaphoreValues.push_back(value);
	}

	// TODO: Maybe make thread-safe
	void VulkanQueue::Submit(const VulkanCommandList** cmds, uint64 numCmd)
	{
		std::vector<VkCommandBuffer> cmdBuffers(numCmd);
		std::vector<VkPipelineStageFlags> waitStages(m_WaitSemaphores.size());

		for (int32 i = 0; i < m_WaitSemaphores.size(); i++)
		{
			waitStages[i] = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		}
		m_LastSubmitID++;

		for (uint64 i = 0; i < numCmd; i++)
		{
			VulkanCommandList* list = (VulkanCommandList*)cmds[i];
			cmdBuffers[i] = list->GetCmdHandle();
			m_ListsInFlight.push_back(list);
		}

		m_SignalSemaphores.push_back(m_TrackingSemaphore);
		m_SignalSemaphoreValues.push_back(m_LastSubmitID);

		VkTimelineSemaphoreSubmitInfo semaphoreInfo{ .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO };
		semaphoreInfo.signalSemaphoreValueCount = m_SignalSemaphoreValues.size();
		semaphoreInfo.pSignalSemaphoreValues = m_SignalSemaphoreValues.data();

		if (!m_WaitSemaphoreValues.empty())
		{
			semaphoreInfo.waitSemaphoreValueCount = m_WaitSemaphoreValues.size();
			semaphoreInfo.pWaitSemaphoreValues = m_WaitSemaphoreValues.data();
		}

		VkSubmitInfo submitInfo{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO };
		submitInfo.pNext = &semaphoreInfo;
		submitInfo.commandBufferCount = numCmd;
		submitInfo.pCommandBuffers = cmdBuffers.data();
		submitInfo.waitSemaphoreCount = m_WaitSemaphores.size();
		submitInfo.pWaitSemaphores = m_WaitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();
		submitInfo.signalSemaphoreCount = m_SignalSemaphores.size();
		submitInfo.pSignalSemaphores = m_SignalSemaphores.data();

		VK_CHECK(vkQueueSubmit(m_Queue, 1, &submitInfo, nullptr));

		m_WaitSemaphores.clear();
		m_WaitSemaphoreValues.clear();
		m_SignalSemaphores.clear();
		m_SignalSemaphoreValues.clear();

		//return m_LastSubmitID;
	}

	//void VulkanQueue::WaitCommandList(VulkanCommandList* cmd, uint64 timeout)
	//{
    //
	//}

	uint64 VulkanQueue::UpdateLastFinishedID() 
	{
		VK_CHECK(vkGetSemaphoreCounterValue(m_Device.GetDeviceHandle(), m_TrackingSemaphore, &m_LastFinishedID));
		return m_LastFinishedID;
	}

	void VulkanQueue::RetireCommandLists()
	{
		std::vector<TRefCountPtr<VulkanCommandList>> submissions{};
		std::swap(submissions, m_ListsInFlight);

		UpdateLastFinishedID();

		for (auto& cmd : submissions)
		{
			if (cmd->SubmissionID <= m_LastFinishedID)
			{
				cmd->SubmissionID = 0;
				m_ListsPool.push_back(cmd);
			}
			else
			{
				m_ListsInFlight.push_back(cmd);
			}
		}
	}

	bool VulkanQueue::PollCommandList(uint64 cmdID)
	{
		if (cmdID > m_LastSubmitID || cmdID == 0)
			return false;

		bool completed = m_LastFinishedID >= cmdID;
		if (completed)
			return true;

		completed = UpdateLastFinishedID() >= cmdID;
		return completed;
	}

	bool VulkanQueue::WaitCommandList(VulkanCommandList* cmd, uint64 timeout)
	{
		if (cmd->SubmissionID > m_LastSubmitID || cmd->SubmissionID == 0)
			return false;

		if (PollCommandList(cmd->SubmissionID))
			return true;

		VkSemaphoreWaitInfo info{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO };
		info.pSemaphores = &m_TrackingSemaphore;
		info.pValues = &cmd->SubmissionID;
		info.semaphoreCount = 1;

		VkResult result = vkWaitSemaphores(m_Device.GetDeviceHandle(), &info, timeout);
		return result == VK_SUCCESS;
	}

	/*
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
		VkImageSubresourceRange clearRange{};
		clearRange.baseArrayLayer = range.BaseLayer;
		clearRange.baseMipLevel = range.BaseMip;
		clearRange.layerCount = range.NumLayers;
		clearRange.levelCount = range.NumMips;
		clearRange.aspectMask = ConvertVkImageAspect(tex->GetFormat());

		vkCmdClearColorImage(m_CmdBuffer, vkTex, ConvertVkImageLayout(access), &clearValue, 1, &clearRange);
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

		vkRegion.imageSubresource.aspectMask = ConvertVkImageAspect(src->GetFormat());
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
			b.srcStageMask = ConvertVkPipelineStage(region.LastAccess);
			b.srcAccessMask = ConvertVkResourceState(region.LastAccess);
			b.oldLayout = ConvertVkImageLayout(region.LastAccess);
			b.dstStageMask = ConvertVkPipelineStage(region.NewAccess);
			b.dstAccessMask = ConvertVkResourceState(region.NewAccess);
			b.newLayout = ConvertVkImageLayout(region.NewAccess);
			b.subresourceRange.baseArrayLayer = region.Range.BaseLayer;
			b.subresourceRange.baseMipLevel = region.Range.BaseMip;
			b.subresourceRange.layerCount = region.Range.NumLayers;
			b.subresourceRange.levelCount = region.Range.NumMips;
			b.subresourceRange.aspectMask = ConvertVkImageAspect(texture->GetFormat());

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
		barrier.srcStageMask = ConvertVkPipelineStage(lastAccess);
		barrier.srcAccessMask = ConvertVkResourceState(lastAccess);
		barrier.dstStageMask = ConvertVkPipelineStage(newAccess);
		barrier.dstAccessMask = ConvertVkResourceState(newAccess);
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
	*/

	VulkanBuffer::VulkanBuffer(uint64 size, EBufferFlags flags, VulkanRHIDevice& device)
		: RHIBuffer(size, flags), m_Device(device), m_MappedData(nullptr)
	{
		VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.pNext = nullptr;
		bufferInfo.flags = 0;
		bufferInfo.size = size;
		bufferInfo.usage = 0;

		if (EnumHasAllFlags(flags, EBufferFlags::Constant))
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		}
		if (EnumHasAllFlags(flags, EBufferFlags::Index))
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		}
		if (EnumHasAllFlags(flags, EBufferFlags::Indirect))
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		}
		if (EnumHasAllFlags(flags, EBufferFlags::Storage))
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		}
		if (EnumHasAllFlags(flags, EBufferFlags::GPUAddress))
		{
			bufferInfo.usage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}

		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		VmaAllocationCreateInfo allocInfo{};
		if (EnumHasAllFlags(flags, EBufferFlags::ReadBack))
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}
		else if (EnumHasAllFlags(flags, EBufferFlags::Upload))
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
			allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
		}
		else 
		{
			allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		}

		VK_CHECK(vmaCreateBuffer(m_Device.GetAllocatorHandle(), &bufferInfo, &allocInfo, &m_Buffer,
			&m_Allocation, nullptr));

		if (EnumHasAnyFlags(flags, EBufferFlags::Upload | EBufferFlags::ReadBack)) 
		{
			vmaMapMemory(m_Device.GetAllocatorHandle(), m_Allocation, &m_MappedData);
		}
	}

	VulkanBuffer::~VulkanBuffer() 
	{
		m_Device.DestroyResource({ 
			.Buffer = m_Buffer, 
			.Allocation = m_Allocation
			});
	}

	VulkanTexture::VulkanTexture(const TextureDesc& desc, VulkanRHIDevice& device) 
		: RHITexture(desc), m_Device(device)
	{
		VkImageCreateInfo imgInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imgInfo.format = ConvertVkImageFormat(desc.Format);
		imgInfo.extent = VkExtent3D{ desc.Width, desc.Height, desc.Depth };
		imgInfo.mipLevels = desc.MipLevels;
		imgInfo.arrayLayers = desc.ArraySize;
		imgInfo.samples = (VkSampleCountFlagBits)desc.SampleCount;
		imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imgInfo.usage = 0;
		imgInfo.flags = 0;

		switch (desc.Dimension)
		{
		case ETextureDimension::Texture1D:
		case ETextureDimension::Texture1DArray:
			imgInfo.imageType = VK_IMAGE_TYPE_1D;
		case ETextureDimension::Texture2D:
		case ETextureDimension::Texture2DArray:
			imgInfo.imageType = VK_IMAGE_TYPE_2D;
		case ETextureDimension::TextureCube:
		case ETextureDimension::TextureCubeArray:
			imgInfo.imageType = VK_IMAGE_TYPE_2D;
			imgInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		case ETextureDimension::Texture3D:
			imgInfo.imageType = VK_IMAGE_TYPE_3D;
		default:
			assert(0);
			break;
		}

		if (EnumHasAllFlags(desc.Flags, ETextureFlags::Sampled))
		{
			imgInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (EnumHasAllFlags(desc.Flags, ETextureFlags::Storage))
		{
			imgInfo.usage |= VK_IMAGE_USAGE_STORAGE_BIT;
		}
		if (EnumHasAllFlags(desc.Flags, ETextureFlags::ColorTarget))
		{
			imgInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (EnumHasAllFlags(desc.Flags, ETextureFlags::DepthTarget))
		{
			imgInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		if (EnumHasAllFlags(desc.Flags, ETextureFlags::CopySrc))
		{
			imgInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}
		if (EnumHasAllFlags(desc.Flags, ETextureFlags::CopyDst))
		{
			imgInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		VK_CHECK(vmaCreateImage(m_Device.GetAllocatorHandle(), &imgInfo, &allocInfo, &m_Image, &m_Allocation, nullptr));
	}

	VulkanTexture::~VulkanTexture() 
	{
		m_Device.DestroyResource({ 
			.Image = m_Image, 
			.Allocation = m_Allocation 
			});

		for (auto& [k, v] : m_ViewsMap) 
		{
			m_Device.DestroyResource({ .View = v });
		}
		m_ViewsMap.clear();
	}

	VkImageView VulkanTexture::GetSubresourceView(const TextureSubresourceSet& subresource, ETextureDimension dimension,
		ETextureFormat format, ESubresourceViewType viewType)
	{
		std::lock_guard lock(m_Mutex);

		if (dimension == ETextureDimension::None)
			dimension = m_Desc.Dimension;

		if (format == ETextureFormat::None)
			format = m_Desc.Format;

		SubresourceViewKey key{};
		key.Dimension = dimension;
		key.Format = format;
		key.Subresources = subresource;
		key.ViewType = viewType;

		auto it = m_ViewsMap.find(key);
		if (it != m_ViewsMap.end()) {
			return it->second;
		}

		VkImageViewCreateInfo viewInfo{ .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = m_Image;
		viewInfo.format = ConvertVkImageFormat(format);
		viewInfo.subresourceRange.baseArrayLayer = subresource.BaseLayer;
		viewInfo.subresourceRange.baseMipLevel = subresource.BaseMip;
		viewInfo.subresourceRange.layerCount = subresource.NumLayers;
		viewInfo.subresourceRange.levelCount = subresource.NumMips;
		viewInfo.subresourceRange.aspectMask = ConvertVkImageAspect(format);

		if ((viewInfo.subresourceRange.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
			== (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))
		{
			if (viewType == ESubresourceViewType::DepthOnly)
			{
				viewInfo.subresourceRange.aspectMask &= (~VK_IMAGE_ASPECT_STENCIL_BIT);
			}
			else if (viewType == ESubresourceViewType::StencilOnly)
			{
				viewInfo.subresourceRange.aspectMask &= (~VK_IMAGE_ASPECT_DEPTH_BIT);
			}
		}

		switch (dimension)
		{
		case ETextureDimension::Texture1D:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
			break;
		case ETextureDimension::Texture1DArray:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			break;
		case ETextureDimension::Texture2D:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			break;
		case ETextureDimension::Texture2DArray:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			break;
		case ETextureDimension::TextureCube:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			break;
		case ETextureDimension::TextureCubeArray:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			break;
		case ETextureDimension::Texture3D:
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
			break;
		default:
			assert(0);
			break;
		}

		VkImageView& view = m_ViewsMap[key];
		VK_CHECK(vkCreateImageView(m_Device.GetDeviceHandle(), &viewInfo, nullptr, &view));

		return view;
	}

	VulkanSamplerState::VulkanSamplerState(const SamplerStateDesc& desc, VulkanRHIDevice& device) 
		: RHISamplerState(desc), m_Device(device)
	{
		VkSamplerCreateInfo info{ .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

		switch (desc.Filter)
		{
		case ESamplerFilter::Point:
			info.magFilter = VK_FILTER_NEAREST;
			info.minFilter = VK_FILTER_NEAREST;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case ESamplerFilter::Bilinear:
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case ESamplerFilter::Trilinear:
			info.magFilter = VK_FILTER_LINEAR;
			info.minFilter = VK_FILTER_LINEAR;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			info.magFilter = VK_FILTER_NEAREST;
			info.minFilter = VK_FILTER_NEAREST;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}

		info.addressModeU = ConvertVkAddressMode(desc.AddressU);
		info.addressModeV = ConvertVkAddressMode(desc.AddressV);
		info.addressModeW = ConvertVkAddressMode(desc.AddressW);
		info.minLod = desc.MinLOD;
		info.maxLod = desc.MaxLOD;
		info.anisotropyEnable = false;
		info.maxAnisotropy = (float)desc.MaxAnisotropy;

		VkSamplerReductionModeCreateInfoEXT createInfoReduction{};
		if (desc.Reduction != ESamplerReduction::Standard)  
		{
			createInfoReduction.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO_EXT;
			createInfoReduction.reductionMode = ConvertVkReductionMode(desc.Reduction);
			info.pNext = &createInfoReduction;
		}

		VK_CHECK(vkCreateSampler(m_Device.GetDeviceHandle(), &info, nullptr, &m_Sampler));
	}

	VulkanSamplerState::~VulkanSamplerState() 
	{
		// FIXME
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

				for (auto& x : shaderBindings) 
				{
					bool found = false;

					for (auto& y : m_LayoutBindings) 
					{
						if (x.binding == y.binding) 
						{
							// check for overlapping descriptors
							assert(x.descriptorCount == y.descriptorCount
								&& x.descriptorType == y.descriptorType);

							y.stageFlags |= x.stageFlags;
							found = true;
							break;
						}
					}

					if (!found) 
					{
						m_LayoutBindings.push_back(x);
					}
				}

				auto pushConstants = vkShader->GetPushConstants();
				if (pushConstants.size > 0) 
				{
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
			for (auto& x : m_LayoutBindings) 
			{
				layoutHash.Bindings.push_back(x);
			}

			layoutHash.PushConstants = m_PushConstants;
			layoutHash.ComputeHash();

			VulkanPSOLayout cachedLayout = m_Device.CreateCachedPSOLayout(layoutHash);
			m_Layout = cachedLayout.Layout;
			m_SetLayout = cachedLayout.SetLayout;
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

			VK_CHECK(vkCreateComputePipelines(m_Device.GetDeviceHandle(), m_Device.GetPipelineCacheHandle(),
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
			inputInfo.topology = ConvertVkPrimitiveTopology(desc.PrimitiveTopology);
			inputInfo.primitiveRestartEnable = false;

			VkPipelineRasterizationStateCreateInfo rasterInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterInfo.polygonMode = desc.Wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
			rasterInfo.lineWidth = 1.f;
			rasterInfo.cullMode = ConvertVkCullMode(desc.CullMode);
			rasterInfo.frontFace = ConvertVkFrontFace(desc.FrontFace);

			VkFormat                            rtFormats[8] = {};
			VkPipelineColorBlendAttachmentState blendStates[8] = {};

			for (uint8 i = 0; i < desc.NumRenderTargets; i++) 
			{
				auto& rt = desc.RenderTargets[i];

				blendStates[i].blendEnable = rt.EnableBlend;
				blendStates[i].srcColorBlendFactor = ConvertVkBlendFactor(rt.SrcBlend);
				blendStates[i].dstColorBlendFactor = ConvertVkBlendFactor(rt.DstBlend);
				blendStates[i].colorBlendOp = ConvertVkBlendOp(rt.BlendOp);
				blendStates[i].srcAlphaBlendFactor = ConvertVkBlendFactor(rt.SrcBlendAlpha);
				blendStates[i].dstAlphaBlendFactor = ConvertVkBlendFactor(rt.DstBlendAlpha);
				blendStates[i].alphaBlendOp = ConvertVkBlendOp(rt.BlendOpAlpha);
				blendStates[i].colorWriteMask = (uint32)rt.ColorMask;

				rtFormats[i] = ConvertVkImageFormat(rt.Format);
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
			renderingInfo.depthAttachmentFormat = ConvertVkImageFormat(desc.DepthFormat);

			// TODO: add stencil support
			renderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			depthStencilInfo.depthTestEnable = desc.DepthEnable;
			depthStencilInfo.depthWriteEnable = desc.DepthWriteEnable;
			depthStencilInfo.depthCompareOp = ConvertVkCompareOp(desc.DepthFunc);
			depthStencilInfo.depthBoundsTestEnable = desc.DepthClipEnable;
			depthStencilInfo.stencilTestEnable = desc.StencilEnable;
			
			if (desc.StencilEnable) 
			{
				depthStencilInfo.front.failOp = ConvertVkStencilOp(desc.FrontStencil.FailOp);
				depthStencilInfo.front.passOp = ConvertVkStencilOp(desc.FrontStencil.PassOp);
				depthStencilInfo.front.depthFailOp = ConvertVkStencilOp(desc.FrontStencil.DepthFailOp);
				depthStencilInfo.front.compareOp = ConvertVkCompareOp(desc.FrontStencil.Func);
				depthStencilInfo.front.compareMask = desc.FrontStencil.ReadMask;
				depthStencilInfo.front.writeMask = desc.FrontStencil.WriteMask;
				depthStencilInfo.front.reference = 0;

				depthStencilInfo.back.failOp = ConvertVkStencilOp(desc.BackStencil.FailOp);
				depthStencilInfo.back.passOp = ConvertVkStencilOp(desc.BackStencil.PassOp);
				depthStencilInfo.back.depthFailOp = ConvertVkStencilOp(desc.BackStencil.DepthFailOp);
				depthStencilInfo.back.compareOp = ConvertVkCompareOp(desc.BackStencil.Func);
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

			VK_CHECK(vkCreateGraphicsPipelines(m_Device.GetDeviceHandle(), m_Device.GetPipelineCacheHandle(), 1, &pipelineInfo, nullptr, &m_Pipeline));
		}
	}

	VulkanPipelineState::~VulkanPipelineState() 
	{
	}
}