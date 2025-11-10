cmake_minimum_required(VERSION 3.20)

project(ImGui)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB PRJ_SOURCE CONFIGURE_DEPENDS 
"${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGui/*.cpp"
"${CMAKE_CURRENT_SOURCE_DIR}/Include/ImGui/*.h")

add_library(ImGui STATIC "${PRJ_SOURCE}")
# target_include_directories(ImGui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include/")