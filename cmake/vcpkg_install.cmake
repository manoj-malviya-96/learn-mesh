

function(setup_triplet)
    set(_os "${CMAKE_SYSTEM_NAME}")
    if(NOT _os)
        set(_os "${CMAKE_HOST_SYSTEM_NAME}")
    endif()
    message(STATUS "Detected OS: ${_os}")
    if(_os STREQUAL "Darwin")
        set(VCPKG_TARGET_TRIPLET "arm64-osx-release")
    elseif(_os STREQUAL "Linux")
        set(VCPKG_TARGET_TRIPLET "x64-linux-release")
    elseif(_os STREQUAL "Windows")
        set(VCPKG_TARGET_TRIPLET "x64-windows-release")
    else()
        message(FATAL_ERROR "Unsupported OS for vcpkg triplet - ${_os}")
    endif()
    message(STATUS "Using vcpkg target triplet: ${VCPKG_TARGET_TRIPLET}")
endfunction()


find_program(SYSTEM_VCPKG NAMES vcpkg)
if (NOT SYSTEM_VCPKG OR NOT EXISTS "${SYSTEM_VCPKG}")
    message(STATUS "vcpkg not found in PATH")
    return()
endif()

message(STATUS "Using system-installed vcpkg: ${SYSTEM_VCPKG}")
set(VCPKG_BINARY "${SYSTEM_VCPKG}")
get_filename_component(VCPKG_ROOT "${VCPKG_BINARY}/.." ABSOLUTE)

set(VCPKG_ROOT "${VCPKG_ROOT}")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

setup_triplet()
set(VCPKG_FEATURE_FLAGS "binarycaching")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_PROGRESS_REPORT ON)
set(VCPKG_VERBOSE "1" CACHE STRING "Enable verbose output from vcpkg" FORCE)