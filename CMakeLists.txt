cmake_minimum_required(VERSION 3.25)

option(USE_TRACY_PROFILER "Enable Tracy profiler" ON)
option(USE_AVX2 "Enable AVX2" OFF)

project(Spikey3D-Engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    set(PLATFORM_NAME "Win32")
else()
    message(FATAL_ERROR "Non Win32 platforms are not yet supported!")
endif()

set(SPIKEY_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(BINARIES_DIR "${SPIKEY_ROOT_DIR}/Binaries/${PLATFORM_NAME}")
set(THIRD_PARTY_DIR "${SPIKEY_ROOT_DIR}/Source/ThirdParty")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# taken from https://github.com/TheLartians/GroupSourcesByFolder.cmake/blob/master/CMakeLists.txt
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

add_subdirectory(Source)