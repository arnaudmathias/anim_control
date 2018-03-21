cmake_minimum_required(VERSION 3.5)
project(anim_control)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)

find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(third-party/glfw)
include_directories(third-party/glfw/include)
include_directories(third-party/glad)
include_directories(third-party/glm)
include_directories(third-party/stb)
include_directories(third-party)
include_directories(${OPENGL_INCLUDE_DIR})

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(SOURCE_FILES
        src/main.cpp
        src/env.cpp
        src/game.cpp
        src/shader.cpp
        src/renderer.cpp
        src/camera.cpp
        src/vao.cpp
        src/texture.cpp
        src/culling.cpp
        src/io.cpp
        third-party/glad/glad.c)

add_executable(anim_control ${SOURCE_FILES})

target_link_libraries(anim_control glfw ${GLFW_LIBRARIES})
