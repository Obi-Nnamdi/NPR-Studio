cmake_minimum_required(VERSION 3.0)
project(GLOO_external)
set(ASSIGNMENT_ID "5")

set(CMAKE_CXX_STANDARD 11)
if (MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected; default to release.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(cxx_warning_flags "-Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(cxx_warning_flags "-Wall")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVCU")
    set(cxx_warning_flags "/W4")
endif()

# Build external libraries and set dependencies
include(ExternalProject)
set(install_dir ${PROJECT_SOURCE_DIR}/install/)
set(source_dir ${PROJECT_SOURCE_DIR}/src/)

# GLFW
ExternalProject_Add(
    glfw3_external
    SOURCE_DIR "${source_dir}/glfw-3.3.2/"
    CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${install_dir}"
        "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
        "-DGLFW_BUILD_DOCS=OFF"
        "-DGLFW_BUILD_TESTS=OFF"
        "-DGLFW_BUILD_EXAMPLES=OFF"
        "-DGLFW_INSTALL=ON"
)

