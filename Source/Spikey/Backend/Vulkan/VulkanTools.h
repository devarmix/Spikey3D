#pragma once

#include <Backend/Vulkan/VulkanCommon.h>
#include <Engine/Graphics/Texture.h>
#include <Engine/Graphics/Shader.h>

namespace Spikey::Vulkan
{
	class VulkanTools
	{
	private:
        static VkFormat PixelFormatToVkFormat[(uint32)PixelFormat::MAX];

	public:
		static VkPipelineStageFlags2 GetBufferBarrierFlags(VkAccessFlags2 accessFlags)
		{
            VkPipelineStageFlags stageFlags = (VkPipelineStageFlags)0;
            switch (accessFlags)
            {
            case 0:
                stageFlags = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                break;
            case VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
                break;
            case VK_ACCESS_2_TRANSFER_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                break;
            case VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_ACCESS_2_TRANSFER_READ_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                break;
            case VK_ACCESS_2_SHADER_READ_BIT:
            case VK_ACCESS_2_SHADER_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                break;
            case VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                break;
            case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT:
            case VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT:
                stageFlags = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                break;
            default:
                assert(0);
                break;
            }

            return stageFlags;
		}

		static void GetImageBarrierFlags(VkImageLayout layout, VkPipelineStageFlags2& stageFlags, VkAccessFlags2& accessFlags)
		{
            switch (layout)
            {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                accessFlags = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                accessFlags = VK_ACCESS_2_TRANSFER_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
                accessFlags = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                accessFlags = VK_ACCESS_2_TRANSFER_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                accessFlags = VK_ACCESS_2_NONE;
                stageFlags = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                accessFlags = VK_ACCESS_2_SHADER_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
                accessFlags = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
                accessFlags = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                accessFlags = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT;
                stageFlags = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
                break;
            default:
                assert(0);
                break;
            }
		}

		static VkFormat ToVulkanFormat(PixelFormat format)
		{
            return PixelFormatToVkFormat[(uint32)format];
		}

		static VkBlendOp ToVulkanBlendOp(BlendOperation op)
		{
            switch (op)
            {
            case BlendOperation::Add:
                return VK_BLEND_OP_ADD;
            case BlendOperation::Subtract:
                return VK_BLEND_OP_SUBTRACT;
            case BlendOperation::ReverseSubtract:
                return VK_BLEND_OP_REVERSE_SUBTRACT;
            case BlendOperation::Min:
                return VK_BLEND_OP_MIN;
            case BlendOperation::Max:
                return VK_BLEND_OP_MAX;
            default:
                return VK_BLEND_OP_ADD;
            }
		}

		static VkCompareOp ToVulkanCompareOp(ComparisonFunc comp)
		{
            switch (comp)
            {
            case ComparisonFunc::Never:
                return VK_COMPARE_OP_NEVER;
            case ComparisonFunc::Less:
                return VK_COMPARE_OP_LESS;
            case ComparisonFunc::Equal:
                return VK_COMPARE_OP_EQUAL;
            case ComparisonFunc::LessOrEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case ComparisonFunc::Greater:
                return VK_COMPARE_OP_GREATER;
            case ComparisonFunc::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case ComparisonFunc::GreaterOrEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case ComparisonFunc::Always:
                return VK_COMPARE_OP_ALWAYS;
            default:
                return VK_COMPARE_OP_NEVER;
            }
		}

        static VkBlendFactor ToVulkanBlendFactor(BlendFactor factor)
        {
            switch (factor)
            {
            case BlendFactor::Zero:
                return VK_BLEND_FACTOR_ZERO;
            case BlendFactor::One:
                return VK_BLEND_FACTOR_ONE;
            case BlendFactor::SrcColor:
                return VK_BLEND_FACTOR_SRC_COLOR;
            case BlendFactor::OneMinusSrcColor:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case BlendFactor::SrcAlpha:
                return VK_BLEND_FACTOR_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstAlpha:
                return VK_BLEND_FACTOR_DST_ALPHA;
            case BlendFactor::OneMinusDstAlpha:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            case BlendFactor::DstColor:
                return VK_BLEND_FACTOR_DST_COLOR;
            case BlendFactor::OneMinusDstColor:
                return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case BlendFactor::SrcAlphaSaturate:
                return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
            case BlendFactor::ConstantColor:
                return VK_BLEND_FACTOR_CONSTANT_COLOR;
            case BlendFactor::OneMinusConstantColor:
                return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
            case BlendFactor::Src1Color:
                return VK_BLEND_FACTOR_SRC1_COLOR;
            case BlendFactor::OneMinusSrc1Color:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
            case BlendFactor::Src1Alpha:
                return VK_BLEND_FACTOR_SRC1_ALPHA;
            case BlendFactor::OneMinusSrc1Alpha:
                return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
            default:
                return VK_BLEND_FACTOR_ZERO;
            }
        }

        static VkSamplerAddressMode ToVulkanAddressMode(SamplerAddressMode mode)
        {
            switch (mode)
            {
            case SamplerAddressMode::Wrap:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerAddressMode::Clamp:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::Mirror:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            default:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            }
        }

        static VkStencilOp ToVulkanStencilOp(StencilOp op)
        {
            switch (op)
            {
            case StencilOp::Keep:
                return VK_STENCIL_OP_KEEP;
            case StencilOp::Zero:
                return VK_STENCIL_OP_ZERO;
            case StencilOp::Replace:
                return VK_STENCIL_OP_REPLACE;
            case StencilOp::IncrementAndClamp:
                return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
            case StencilOp::DecrementAndClamp:
                return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
            case StencilOp::Invert:
                return VK_STENCIL_OP_INVERT;
            case StencilOp::IncrementAndWrap:
                return VK_STENCIL_OP_INCREMENT_AND_WRAP;
            case StencilOp::DecrementAndWrap:
                return VK_STENCIL_OP_DECREMENT_AND_WRAP;
            default:
                return VK_STENCIL_OP_KEEP;
            }
        }
	};
}