#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#include <Engine/Core/Common.h>

#define VK_CHECK(x)                                                                \
    do {                                                                           \
        VkResult err = x;                                                        \
        if(err)  {                                                                 \
            ENGINE_ERROR("Detected Vulkan Error: {}", string_VkResult(err));       \
            abort();                                                               \
        }                                                                          \
    } while (0)