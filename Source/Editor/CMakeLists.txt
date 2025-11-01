cmake_minimum_required(VERSION 3.20)

project(Editor)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
"${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(Editor "${SOURCES}")
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES "${SOURCES}")

set_target_properties(
    Editor
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OutputDir}/Editor"
)

target_include_directories(Editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(Editor PRIVATE Spikey)