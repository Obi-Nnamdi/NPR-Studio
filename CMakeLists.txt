cmake_minimum_required(VERSION 3.6)
project(NPR_STUDIO)
message(STATUS "Building NPR Studio...")

# Otherwise IMGUI may pick GLEW as GL loader
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DIMGUI_IMPL_OPENGL_LOADER_GLAD")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
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

message("Using CXX compiler: ${CMAKE_CXX_COMPILER}")
message("             flags: ${CMAKE_CXX_FLAGS}")

# Allow custom CMake configurations.
include(${PROJECT_SOURCE_DIR}/CMakeCustomLists.txt OPTIONAL)

# Allow personal CMake configurations.
include(${PROJECT_SOURCE_DIR}/CMakePersonalLists.txt OPTIONAL)

# Set dependencies for external libraries.
set(external_install_dir ${PROJECT_SOURCE_DIR}/external/install/)
set(external_source_dir ${PROJECT_SOURCE_DIR}/external/src/)

set(external_libs "")
set(external_srcs "")

# GLFW
find_package(
    glfw3
    QUIET
    PATHS ${external_install_dir}/lib/cmake/glfw3
    NO_DEFAULT_PATH
)
if (NOT glfw3_FOUND)
    message(STATUS "No installed GLFW found in ${external_install_dir}.
    Include in build.")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(${external_source_dir}/glfw-3.3.2)
else()
    message(STATUS "Found GLFW installed in ${external_install_dir}.")
endif()
list(APPEND external_libs glfw)

# GLAD
include_directories(${external_source_dir}/glad/include)
list(APPEND external_srcs ${external_source_dir}/glad/src/glad.c)

# GLM
find_package(
    glm
    REQUIRED
    PATHS ${external_source_dir}/glm-0.9.9.8/cmake/glm
    NO_DEFAULT_PATH
)
list(APPEND external_libs glm::glm)

# ImGui
set(imgui_dir ${external_source_dir}/imgui)
list(APPEND external_srcs
    ${imgui_dir}/imgui.cpp
    ${imgui_dir}/imgui_demo.cpp
    ${imgui_dir}/imgui_draw.cpp
    ${imgui_dir}/imgui_widgets.cpp
    ${imgui_dir}/examples/imgui_impl_glfw.cpp
    ${imgui_dir}/examples/imgui_impl_opengl3.cpp)

include_directories(${imgui_dir} ${imgui_dir}/examples)

# stb
include_directories(${external_source_dir}/stb)

###################################################
# Add path macros.
set(gloo_dir ${PROJECT_SOURCE_DIR}/gloo)

###################################################

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB gloo_srcs
    ${gloo_dir}/*.cpp
    ${gloo_dir}/gl_wrapper/*.cpp
    ${gloo_dir}/components/*.cpp
    ${gloo_dir}/shaders/*.cpp
    ${gloo_dir}/parsers/*.cpp
    ${gloo_dir}/cameras/*.cpp
    ${gloo_dir}/debug/*.cpp
)

###################################################

set(assignment_name "npr_studio")
set(assignment_dir ${PROJECT_SOURCE_DIR}/main_code/${assignment_name})
set(assignment_common_dir ${PROJECT_SOURCE_DIR}/main_code/common)
include_directories(${assignment_dir})
include_directories(${assignment_common_dir})
file(GLOB_RECURSE assignment_srcs
    ${assignment_dir}/*.cpp
    ${assignment_common_dir}/*.cpp)

file(GLOB header_files
    ${gloo_dir}/*.hpp
    ${gloo_dir}/*/*.hpp
    ${assignment_dir}/*.hpp
    ${assignment_dir}/*/*.hpp
    ${imgui_dir}/*.hpp
    ${imgui_dir}/*/*.hpp
)

set(all_files ${assignment_srcs};${external_srcs};${gloo_srcs};${header_files})

foreach (source IN LISTS all_files)
    file(RELATIVE_PATH source_rel ${CMAKE_CURRENT_LIST_DIR} ${source})
    get_filename_component(source_path "${source_rel}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach ()

add_executable(${assignment_name} ${gloo_srcs} ${external_srcs} ${assignment_srcs} ${header_files})

target_link_libraries(${assignment_name} ${external_libs})
target_compile_options(${assignment_name} PRIVATE ${cxx_warning_flags})

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${assignment_name})
endif ()

