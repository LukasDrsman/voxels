cmake_minimum_required(VERSION 3.30)
project(voxels)

set(CMAKE_CXX_STANDARD 23)

include_directories(.)

find_package(glm REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)


add_executable(voxels
        irenderable.hpp
        main.cpp
        voxel.hpp
        window.hpp
        shaders.hpp
        voxel_scene.hpp
        voxel_linearized.hpp
)

target_link_libraries(voxels
        glm::glm
        OpenGL::GL
        GLEW
        OpenGL::GLES3
        glfw
)


