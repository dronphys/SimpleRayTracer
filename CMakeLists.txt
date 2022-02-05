cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
set(PROJECT_NAME ComputeShaders)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME} src/main.cpp src/shader.h src/renderer.h
    src/imageHandler.h src/CameraMoves.h)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)


add_subdirectory(external/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

add_subdirectory(external/glad)
target_link_libraries(${PROJECT_NAME} glad)

add_subdirectory(external/imgui)
target_link_libraries(${PROJECT_NAME} imgui)



add_library(stb INTERFACE)
target_include_directories(stb INTERFACE external/stb)
target_link_libraries(${PROJECT_NAME} stb)


