cmake_minimum_required(VERSION 3.20)

project(Spikey)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_library(Spikey STATIC "${SOURCES}")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES "${SOURCES}")

set_target_properties(
    Spikey
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${OutputDir}/Spikey"
)

target_include_directories(Spikey PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/")