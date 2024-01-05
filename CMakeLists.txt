cmake_minimum_required(VERSION 3.6)
project(npr_studio VERSION 1.0.0)
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
set (shader_dir ${gloo_dir}/shaders/glsl) 
set (asset_dir ${PROJECT_SOURCE_DIR}/assets)

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

set(assignment_name "npr_studio") # TODO replace with project name
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
# Just for consistency, we set the base output file name of all targets to be the same
set (OUTPUT_NAME ${assignment_name}) # TODO: doesn't work

# Binary executable
set (binary_name ${assignment_name}_bin)

add_executable(${binary_name} ${gloo_srcs} ${external_srcs} ${assignment_srcs} ${header_files})

target_link_libraries(${binary_name} ${external_libs})
target_compile_options(${binary_name} PRIVATE ${cxx_warning_flags})

# TODO: Add assets/shaders in directory around project, but change it to inside the app if it's OSX.

if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${binary_name})
endif ()

# MAC OS App
if (APPLE)
add_executable(${assignment_name} MACOSX_BUNDLE ${gloo_srcs} ${external_srcs} ${assignment_srcs} ${header_files})

target_link_libraries(${assignment_name} ${external_libs})
target_compile_options(${assignment_name} PRIVATE ${cxx_warning_flags})

# After creating our app, set its prefix
set(app_prefix "${assignment_name}.app/Contents") # TODO make OS specific

# Add gloo.cfg file to bundle
set(gloo_file_path "${CMAKE_CURRENT_BINARY_DIR}/${app_prefix}/gloo.cfg")
file(WRITE "${gloo_file_path}" "shaders=Resources/shaders/\nassets=Resources/assets/")
message(STATUS "gloo.cfg file created at '${gloo_file_path}'.")

# Packaging Things for Installation
# TODO: Copy from here: https://github.com/kracejic/cleanCppProject/blob/master/packaging/CMakeLists.txt
set(canonical_name "NPR Studio")
set(resources_dir "${app_prefix}/Resources")


# Generate Mac .dmg

# Put initial executable in package
install(TARGETS ${assignment_name} DESTINATION ./)

# Put shaders in package
# Trailing slash added so we don't include "glsl" folder itself when installing
install(DIRECTORY ${shader_dir}/ DESTINATION ${resources_dir}/shaders)
# Put assets in package, excluding the render assets and application screenshots, but still keep the /renders folder
install(DIRECTORY ${asset_dir}/ DESTINATION ${resources_dir}/assets
        PATTERN "assets/renders/*" EXCLUDE PATTERN "assets/screenshots" EXCLUDE)

set(gloo_dir ${PROJECT_SOURCE_DIR}/gloo)


# Configure Global CPack Variables
set(CPACK_GENERATOR "DragNDrop") # TODO: also use NSIS
set(CPACK_PACKAGE_NAME ${canonical_name})
set(CPACK_BUNDLE_NAME ${canonical_name})
set(CPACK_PACKAGE_VENDOR "Obi-Nnamdi")
# set(CPACK_PACKAGE_CONTACT "something <something@something.com>") # TODO: Add contact
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(HOMEPAGE "https://github.com/Obi-Nnamdi/NPR-Studio")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# This is the default value for the package file, but for readability we repeat it commented out
# set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME})


# TODO: Add license
# TODO: Add icons

# Tell CPack to put everything we "install" here:
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
# set(CMAKE_INSTALL_PREFIX "${assignment_name}.app/Contents")
# Tell CPack to put our acutal packaging stuff here:
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/out")

# set human names to executables - creates start menu/desktop shortcuts for NSIS
set(CPACK_PACKAGE_EXECUTABLES ${assignment_name} ${canonical_name})
set(CPACK_CREATE_DESKTOP_LINKS ${canonical_name})

# Apple Build Settings
# set(CPACK_PACKAGING_INSTALL_PREFIX "${assignment_name}.app/Contents")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(MACOSX_BUNDLE_BUNDLE_NAME ${CPACK_PACKAGE_NAME}) # Name that shows up in the toolbar when running the app
set(MACOSX_BUNDLE_BUNDLE_GUI_IDENTIFIER "com.example.npr-studio") # TODO handle properly
endif()
# Include cpack
include(CPack)