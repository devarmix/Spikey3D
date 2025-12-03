cmake_minimum_required(VERSION 3.20)

project(Editor)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PRJ_SOURCE CONFIGURE_DEPENDS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(Editor "${PRJ_SOURCE}")
GroupSources(Editor)

set_target_properties(
    Editor
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OutputDir}/Editor"
)

add_custom_command(TARGET Editor POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${ThirdPartyDir}/SDL3/Bin/x64/SDL3.dll" "${CMAKE_BINARY_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${ThirdPartyDir}/DirectXShaderCompiler/Bin/x64/dxcompiler.dll" "${CMAKE_BINARY_DIR}"
)

target_include_directories(Editor PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/")
target_link_libraries(Editor PRIVATE Spikey)