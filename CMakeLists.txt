cmake_minimum_required(VERSION 3.20)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release")
endif()

project(Spikey3D)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OutputDir "${CMAKE_CURRENT_SOURCE_DIR}/Binaries")
set(ThirdPartyDir "${CMAKE_CURRENT_SOURCE_DIR}/Source/ThirdParty")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# taken from https://github.com/TheLartians/GroupSourcesByFolder.cmake/blob/master/CMakeLists.txt
# organizes files in project by folder
function(GroupSources target)
    set(SOURCE_GROUP_DELIMITER "/")
    set(last_dir "")
    set(files "")

    get_target_property(sources ${target} SOURCES)

    foreach(file ${sources})
        file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
        get_filename_component(dir "${relative_file}" PATH)
        if(NOT "${dir}" STREQUAL "${last_dir}")
        if(files)
            source_group("${last_dir}" FILES ${files})
        endif()
            set(files "")
        endif()
            set(files ${files} ${file})
        set(last_dir "${dir}")
    endforeach()

    if(files)
        source_group("${last_dir}" FILES ${files})
    endif()
endfunction()

# third party build dir
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OutputDir}/ThirdParty/Lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OutputDir}/ThirdParty/Lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${OutputDir}/ThirdParty/Bin")

include (FetchContent)

option(USE_VULKAN_RHI "Build engine with Vulkan backend" ON)
option(USE_SDL_BACKEND "Build engine with SDL3 backend" ON)
option(USE_TRACY_PROFILER "Build engine with tracy profiler integrated" ON)

if (WIN32)
    message(STATUS "Building for Win32")
    add_compile_definitions(BUILD_PLATFORM_WIN32)
endif()

if (USE_VULKAN_RHI)
    message(STATUS "Building with Vulkan RHI")
    add_compile_definitions(BUILD_VULKAN_RHI)
endif()

if (USE_SDL_BACKEND)
    message(STATUS "Building with SDL3 backend")
    add_compile_definitions(BUILD_SDL_BACKEND)

    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG main
    )

    # SDL config
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL3)
endif()

if (USE_TRACY_PROFILER)
    message(STATUS "Building with Tracy profiler")
    add_compile_definitions(BUILD_TRACY_PROFILER)

    FetchContent_Declare(
        Tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
    )

    FetchContent_MakeAvailable(Tracy)
endif()

add_subdirectory(Source/ThirdParty/ImGui)
add_subdirectory(Source/Editor)
add_subdirectory(Source/Spikey)