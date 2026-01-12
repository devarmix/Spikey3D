cmake_minimum_required(VERSION 3.20)

project(Spikey)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(
    GLOB_RECURSE PRJ_SOURCE
    CONFIGURE_DEPENDS

    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/SDL/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/SDL/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Vulkan/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Vulkan/*.h"
    "${THIRD_PARTY_DIR}/SPIRV-Reflect/spirv_reflect.cpp"
    "${THIRD_PARTY_DIR}/VkBootstrap/VkBoostrap.cpp"
)

add_library(Spikey STATIC "${PRJ_SOURCE}")
GroupSources(Spikey)

set_target_properties(
    Spikey
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BINARIES_DIR}"
)

target_precompile_headers(
    Spikey
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/Common.h"
)

if (NOT DEFINED "ENV{VULKAN_SDK}")
     message(FATAL_ERROR "No Vulkan SDK found! Please, install latest version of Vulkan SDK 1.4 and try again")
else()
     set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
     message(STATUS "Found Vulkan SDK at: ${VULKAN_SDK_PATH}")
endif()

target_include_directories(
    Spikey
    PUBLIC

    "${CMAKE_CURRENT_SOURCE_DIR}/"
    "${THIRD_PARTY_DIR}/Spdlog/Include/"
    "${THIRD_PARTY_DIR}/ImGui/Include/"
    "${THIRD_PARTY_DIR}/Glm/Include/"
    "${VULKAN_SDK_PATH}/Include/"
    "${THIRD_PARTY_DIR}/VulkanMemoryAllocator/Include/"
    "${THIRD_PARTY_DIR}/SPIRV-Reflect/Include/"
    "${THIRD_PARTY_DIR}/VkBootstrap/Include/"
    "${THIRD_PARTY_DIR}/SDL3/Include/"

    if (SPIKEY_PROFILER)
        "${THIRD_PARTY_DIR}/Tracy/Include/"
    endif()

    if (SPIKEY_EDITOR)
        "${THIRD_PARTY_DIR}/DirectXShaderCompiler/Include/"
    endif()
)

target_link_libraries(
    Spikey
    PUBLIC
    "${THIRD_PARTY_DIR}/SDL3/Lib/x64/SDL3.lib"
    "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib"
    ImGui
)

if (SPIKEY_PROFILER)
    target_link_libraries(
        Spikey
        PUBLIC
        "${THIRD_PARTY_DIR}/Tracy/Lib/x64/TracyClient.lib"
    )
endif()

if (SPIKEY_EDITOR)
    target_link_libraries(
        Spikey
        PUBLIC
        "${THIRD_PARTY_DIR}/DirectXShaderCompiler/Lib/x64/dxcompiler.lib"
    )
endif()