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
)

target_link_libraries(learn_mesh PRIVATE cxxopts::cxxopts)