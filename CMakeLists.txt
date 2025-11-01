cmake_minimum_required(VERSION 3.20)

if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release")
endif()

project(Spikey3D)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OutputDir "${CMAKE_SOURCE_DIR}/Binaries")

add_subdirectory(Source/Editor)
add_subdirectory(Source/Spikey)