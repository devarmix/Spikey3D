cmake_minimum_required(VERSION 3.25)

project(Spikey3D)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SPIKEY_SOURCE CONFIGURE_DEPENDS
    "${SPIKEY_ROOT_DIR}/Source/Engine/*.h"
    "${SPIKEY_ROOT_DIR}/Source/Engine/*.cpp"
    "${SPIKEY_ROOT_DIR}/Source/Platform/*.h"
    "${SPIKEY_ROOT_DIR}/Source/Platform/*.cpp"

    if ("${BUILD_TARGET}" STREQUAL "Editor")
        "${SPIKEY_ROOT_DIR}/Source/Editor/*.h"
        "${SPIKEY_ROOT_DIR}/Source/Editor/*.cpp"
    endif()
)

list(APPEND SPIKEY_SOURCE
    "${THIRD_PARTY_DIR}/ImGui/imgui.cpp"
    "${THIRD_PARTY_DIR}/ImGui/imgui_demo.cpp"
    "${THIRD_PARTY_DIR}/ImGui/imgui_draw.cpp"
    "${THIRD_PARTY_DIR}/ImGui/imgui_tables.cpp"
    "${THIRD_PARTY_DIR}/ImGui/imgui_widgets.cpp"
    "${THIRD_PARTY_DIR}/ImGui/imgui_impl_sdl3.cpp"

    "${THIRD_PARTY_DIR}/SPIRV-Reflect/spirv_reflect.cpp"
    "${THIRD_PARTY_DIR}/VkBootstrap/VkBootstrap.cpp"
)

add_executable(Spikey3D ${SPIKEY_SOURCE})
GroupSources(Spikey3D)

if (NOT DEFINED "ENV{VULKAN_SDK}")
    message(FATAL_ERROR "No Vulkan SDK found! Please, install latest version of Vulkan 1.4 SDK and try again")
else()
    set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
    message(STATUS "Found Vulkan SDK at: ${VULKAN_SDK_PATH}")
endif()

set_target_properties(Spikey3D PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${BINARIES_DIR}/${BUILD_TARGET}")
target_compile_options(Spikey3D PRIVATE "/utf-8")

if (MSVC)
    set_property(TARGET Spikey3D PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
    set_property(TARGET Spikey3D PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    if (USE_AVX2)
        target_compile_definitions(Spikey3D PRIVATE /arch:AVX2)
    endif()
else()
    if (USE_AVX2)
        target_compile_definitions(Spikey3D PRIVATE -mavx2 -mbmi -mpopcnt -mlzcnt -mf16c)
    endif()
endif()

add_custom_command(TARGET Spikey3D POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${THIRD_PARTY_DIR}/SDL3/Bin/x64/SDL3.dll"
    "$<TARGET_FILE_DIR:Spikey3D>"

    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${THIRD_PARTY_DIR}/DirectXShaderCompiler/Bin/x64/dxcompiler.dll"
    "$<TARGET_FILE_DIR:Spikey3D>"
)

target_include_directories(
    Spikey3D PRIVATE
    "${THIRD_PARTY_DIR}/"
    "${SPIKEY_ROOT_DIR}/Source/"
    "${VULKAN_SDK_PATH}/Include/"
)

target_link_libraries(
    Spikey3D PRIVATE
    "${VULKAN_SDK_PATH}/Lib/vulkan-1.lib"
    "${THIRD_PARTY_DIR}/SDL3/Lib/x64/SDL3.lib"
)

if (USE_TRACY_PROFILER)
    target_link_libraries(Spikey3D PRIVATE "${THIRD_PARTY_DIR}/Tracy/Lib/x64/TracyClient.lib")
endif()

target_compile_definitions(
    Spikey3D PRIVATE

    WITH_EDITOR=$<STREQUAL:${BUILD_TARGET},Editor>
    BUILD_SHIPPING=$<BOOL:${BUILD_SHIPPING}>
    WITH_PROFILER=USE_TRACY_PROFILER
    SPIKEY_PLATFORM_WIN32=WIN32
)