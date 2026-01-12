cmake_minimum_required(VERSION 3.20)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release")
endif()

project(Spikey3D)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SPIKEY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
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

option(SPIKEY_PROFILER "Build Spikey with tracy profilter" ON)
option(SPIKEY_EDITOR "Build Spikey editor" ON)

if (WIN32)
    set(PLATFORM_NAME "Win32")
    add_compile_definitions(BUILD_PLATFORM_WIN32)
else()
    message(FATAL_ERROR "Non Win32 platforms are not yet supported!")
endif()

if (SPIKEY_PROFILER)
    add_compile_definitions(BUILD_WITH_PROFILER)
endif()

set(BINARIES_DIR "${SPIKEY_ROOT_DIR}/Binaries/${PLATFORM_NAME}")
set(THIRD_PARTY_DIR "${SPIKEY_ROOT_DIR}/Source/ThirdParty")

add_subdirectory(Source/ThirdParty/ImGui)
add_subdirectory(Source/Spikey)

if (SPIKEY_EDITOR)
    add_subdirectory(Source/Editor)
endif()