cmake_minimum_required(VERSION 3.6)
project(npr_studio VERSION 0.1.0)
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
set (model_dir ${asset_dir}/models)

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

set(all_source_files ${assignment_srcs};${external_srcs};${gloo_srcs};${header_files})

foreach (source IN LISTS all_source_files)
    file(RELATIVE_PATH source_rel ${CMAKE_CURRENT_LIST_DIR} ${source})
    get_filename_component(source_path "${source_rel}" PATH)
    string(REPLACE "/" "\\" source_path_msvc "${source_path}")
    source_group("${source_path_msvc}" FILES "${source}")
endforeach ()

# Get resource files (exclude hidden files)
file(GLOB shader_files ${shader_dir}/[!.]*)
file(GLOB_RECURSE model_files ${asset_dir}/models/[!.]*)
file(GLOB preset_files ${asset_dir}/presets/[!.]*)

# Get icon file
set(icon_file "")
if(WIN32) #windows icon resource file, provided from /packaging directory
    # TODO: Generate windows icon file
    # set(icon_file "${asset_dir}/packaging/npr_studio.icon.rc")
elseif (APPLE) #osx icon ressource file, provided from /packaging directory
    set(icon_file "${asset_dir}/packaging/npr_studio.icns")
endif()

set(resource_files ${shader_files} ${model_files} ${preset_files} ${icon_file})
set_source_files_properties(${resource_files} PROPERTIES HEADER_FILE_ONLY TRUE) # tell compiler to not try and build resource files

# Binary executable
set (binary_name ${assignment_name}_bin)
set (binary_path ${CMAKE_BINARY_DIR}/bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${binary_path}/")

add_executable(${binary_name} ${gloo_srcs} ${external_srcs} ${assignment_srcs} ${header_files} ${resource_files})

target_link_libraries(${binary_name} ${external_libs})
target_compile_options(${binary_name} PRIVATE ${cxx_warning_flags})

# Just for consistency, we set the base output file name of the binary to just be the normal assignment name
set_target_properties(${binary_name} PROPERTIES OUTPUT_NAME ${assignment_name})

# Add resources in directory around executable
set(internal_shader_dir "/Resources/shaders/")
set(internal_asset_dir "/Resources/")
add_custom_target(include_files ALL)
add_custom_command(
        TARGET include_files PRE_BUILD
        # Make Shader, Model, Preset, and Render Dir
        COMMAND ${CMAKE_COMMAND} -E make_directory 
                "${binary_path}/${internal_shader_dir}"
                "${binary_path}/${internal_asset_dir}/presets"
                "${binary_path}/${internal_asset_dir}/models"
                "${binary_path}/${internal_asset_dir}/renders"

        # Copy over shader files
        COMMAND ${CMAKE_COMMAND} -E copy -t
                "${binary_path}/${internal_shader_dir}"
                ${shader_files}

        # Copy over preset files
        COMMAND ${CMAKE_COMMAND} -E copy -t
                "${binary_path}/${internal_asset_dir}/presets/"
                ${preset_files}

        # Copy over model files (use recursive structure)
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${model_dir}
                "${binary_path}/${internal_asset_dir}/models/"
)

# Create gloo.cfg file
file(WRITE "${binary_path}/gloo.cfg" 
           "shaders=${internal_shader_dir}\nassets=${internal_asset_dir}")
if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${binary_name})
endif ()


################################
# Configure Common CPack variables
set(CPACK_PACKAGE_NAME ${canonical_name})
set(CPACK_BUNDLE_NAME ${canonical_name})
set(CPACK_PACKAGE_VENDOR "Obi-Nnamdi")
# set(CPACK_PACKAGE_CONTACT "something <something@something.com>") # TODO: Add contact
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.md")
set(HOMEPAGE "https://github.com/Obi-Nnamdi/NPR-Studio")

set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")

# set(CMAKE_INSTALL_PREFIX "${assignment_name}.app/Contents")
# Tell CPack to put our acutal packaging stuff here:
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")

# set human names to executables - creates start menu/desktop shortcuts for NSIS
set(CPACK_PACKAGE_EXECUTABLES ${assignment_name} ${canonical_name})
set(CPACK_CREATE_DESKTOP_LINKS ${canonical_name})

##################################
# zipping standard executable - done with standard "make"
# This is just done for convenience on Apple machines.
file(ARCHIVE_CREATE
    OUTPUT "${binary_name}.zip"
    PATHS 
        "${binary_path}/"
    FORMAT "zip")

# If we're not on apple (windows or a unix-like system), create a package by just zipping the binary directory (like we just did)
# TODO: add something specific for windows
if (NOT APPLE)
set(CPACK_GENERATOR "TGZ;ZIP")
install(DIRECTORY "${binary_path}" 
        DESTINATION ./
        USE_SOURCE_PERMISSIONS)

# MAC OS App
elseif (APPLE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
add_executable(${assignment_name} MACOSX_BUNDLE ${gloo_srcs} ${external_srcs} ${assignment_srcs} ${header_files} ${resource_files})

target_link_libraries(${assignment_name} ${external_libs})
target_compile_options(${assignment_name} PRIVATE ${cxx_warning_flags})


set(canonical_name "NPR Studio")

# Change name of app to be the canonical name
set_target_properties(${assignment_name} PROPERTIES OUTPUT_NAME "${canonical_name}")

# After creating our app, set its prefix
set(app_prefix "${canonical_name}.app/Contents") # TODO make OS specific
set(resources_dir "${app_prefix}/Resources")

# Configure Global CPack Variables for installation
set(CPACK_GENERATOR "DragNDrop")

# TODO: sign app

# This is the default value for the package file, but for readability we repeat it commented out
# set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME})


# TODO: Add license

# Add icon
set(MACOSX_BUNDLE_ICON_FILE "npr_studio.icns") # don't need to include full path structure here,
                                               # icon file will be looked up in /Resources dir automatically
set_source_files_properties(${icon_file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# Apple Build Settings
set(MACOSX_BUNDLE_BUNDLE_NAME ${CPACK_PACKAGE_NAME}) # Name that shows up in the toolbar when running the app
set(MACOSX_BUNDLE_BUNDLE_GUI_IDENTIFIER "com.example.npr-studio") # TODO handle properly

# Generate Mac .dmg
# Put initial executable in package
install(TARGETS ${assignment_name} DESTINATION ./)

# Add gloo.cfg file to bundle that tells program where resource files are
set(internal_asset_dir "Resources/") # note: causes double slashes in file names while building, but it hasn't been an issue
set(internal_shader_dir "Resources/shaders/")

# TODO: smarter way to do this that covers multiple OS configs?
set(gloo_file_path "${CMAKE_CURRENT_BINARY_DIR}/${app_prefix}/gloo.cfg")
file(WRITE "${gloo_file_path}" "shaders=${internal_shader_dir}\nassets=${internal_asset_dir}")

# Put shaders in package
set_source_files_properties(${shader_files} PROPERTIES MACOSX_PACKAGE_LOCATION "${internal_shader_dir}")

# Put models in package, preserving directory structure
foreach (model_source IN LISTS model_files)
    file(RELATIVE_PATH model_source_rel ${model_dir} ${model_source})
    get_filename_component(rel_folder "${model_source_rel}" PATH)
    set_source_files_properties(${model_source} 
       PROPERTIES 
       MACOSX_PACKAGE_LOCATION "${internal_asset_dir}/models/${rel_folder}")

endforeach ()
# Put presets in package
set_source_files_properties(${preset_files} 
    PROPERTIES 
    MACOSX_PACKAGE_LOCATION "${internal_asset_dir}/presets"
)

# Make an empty directory for renders
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${app_prefix}/${internal_asset_dir}/renders)

set(gloo_dir ${PROJECT_SOURCE_DIR}/gloo)
endif()
# Include cpack
include(CPack)