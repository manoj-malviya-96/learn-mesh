cmake_minimum_required(VERSION 4.0)
include(cmake/vcpkg_install.cmake)


project(learn_mesh)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(cxxopts REQUIRED)
add_executable(learn_mesh
        main.cpp
        src/load_mesh.cpp
        src/load_mesh.h
        src/tri_mesh.h
        src/half_edge.cpp
        src/half_edge.h
        src/simplify_mesh.h
        src/simplify_mesh.cpp
        src/mat4.h
        src/mesh_operations.h
        src/mesh_operations.cpp
)

target_link_libraries(learn_mesh PRIVATE cxxopts::cxxopts)