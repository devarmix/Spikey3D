cmake_minimum_required(VERSION 3.20)

project(Spikey)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PRJ_SOURCE CONFIGURE_DEPENDS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(Spikey STATIC "${PRJ_SOURCE}")
GroupSources(Spikey)

set_target_properties(
    Spikey
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OutputDir}/Spikey"
)

target_precompile_headers(Spikey PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Core/Common.h")
target_include_directories(Spikey PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/"
    "${ThirdPartyDir}/Spdlog/include/"
)

target_link_libraries(Spikey PUBLIC SDL3::SDL3)