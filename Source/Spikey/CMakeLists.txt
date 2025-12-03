cmake_minimum_required(VERSION 3.20)

project(Spikey)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PRJ_SOURCE CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h"
    "${THIRD_PARTY_DIR}/SPIRV-Reflect/spirv_reflect.cpp"

if (USE_SDL_BACKEND)
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/SDL/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/SDL/*.h"
endif()

if (USE_VULKAN_RHI)
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Vulkan/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Vulkan/*.h"
endif()
)

add_library(Spikey STATIC "${PRJ_SOURCE}")
GroupSources(Spikey)

set_target_properties(
    Spikey
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OutputDir}/Spikey"
)

target_precompile_headers(Spikey PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Core/Common.h")
target_include_directories(Spikey PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    "${THIRD_PARTY_DIR}/Spdlog/Include/"
    "${THIRD_PARTY_DIR}/ImGui/Include/"
    "${THIRD_PARTY_DIR}/Glm/Include/"
    "${THIRD_PARTY_DIR}/SPIRV-Reflect/Include/"

    if (USE_VULKAN_RHI)
        "${VULKAN_SDK_PATH}/Include/"
        "${THIRD_PARTY_DIR}/VulkanMemoryAllocator/Include/"
    endif()

    if (USE_SDL_BACKEND)
          "${THIRD_PARTY_DIR}/SDL3/Include/"
    endif()

    if (USE_TRACY_PROFILER)
        "${THIRD_PARTY_DIR}/Tracy/Include/"
    endif()

    if (WITH_EDITOR)
        "${THIRD_PARTY_DIR}/DirectXShaderCompiler/Include/"
    endif()
)

if (USE_SDL_BACKEND)
    target_link_libraries(Spikey PUBLIC "${THIRD_PARTY_DIR}/SDL3/Lib/x64/SDL3.lib")
endif()

if (USE_TRACY_PROFILER)
    target_link_libraries(Spikey PUBLIC "${THIRD_PARTY_DIR}/Tracy/Lib/x64/TracyClient.lib")
endif()

if (USE_VULKAN_RHI)
    target_link_libraries(Spikey PUBLIC "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib")
endif()

if (WITH_EDITOR)
    target_link_libraries(Spikey PUBLIC "${THIRD_PARTY_DIR}/DirectXShaderCompiler/Lib/x64/dxcompiler.lib")
endif()

target_link_libraries(Spikey PUBLIC ImGui)