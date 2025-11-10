cmake_minimum_required(VERSION 3.20)

project(Spikey)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (USE_VULKAN_RHI)
    set(RHI_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Backend/Vulkan")
else()
    set(RHI_MODULE_PATH "")
endif()

if (USE_SDL_BACKEND)
    set(WINDOW_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Backend/SDL")
else()
    set(WINDOW_MODULE_PATH "")
endif()

file(GLOB_RECURSE PRJ_SOURCE CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h"
    "${RHI_MODULE_PATH}/*.cpp"
    "${RHI_MODULE_PATH}/*.h"
    "${WINDOW_MODULE_PATH}/*.cpp"
    "${WINDOW_MODULE_PATH}/*.h"
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
    "${ThirdPartyDir}/Spdlog/Include/"
    "${ThirdPartyDir}/ImGui/Include/"
    "${ThirdPartyDir}/Glm/Include/"
)

if (USE_SDL_BACKEND)
    target_link_libraries(Spikey PUBLIC SDL3::SDL3)
endif()

if (USE_TRACY_PROFILER)
    target_link_libraries(Spikey PUBLIC Tracy::TracyClient)
endif()

target_link_libraries(Spikey PUBLIC ImGui)