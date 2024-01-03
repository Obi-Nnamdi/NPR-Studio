# NPR Studio
A lightweight real-time non-photorealistic renderer built with OpenGL. 

This project started as the final project of MIT's 6.4400 - "Computer Graphics" course, but was extented afterwards.

## Table of Contents

| [Features](#features) - [Gallery](#gallery) -  [Installing](#installing) - [Running](#running) - [Project Writup](#project-writeup)| 
| :----------------------------------------------------------: |
| [High-Level Code Organization](#high-level-code-organization) - [Editing Code](#editing-code) - [Credits](#credits) - [Dependencies](#dependencies)|

## Features

* Opening .obj files (with attached .mtl files supported)
  * Currently, only the diffuse color from .mtl files is used for rendering.
  * If no .mtl file with diffuse colors is specified, then default colors are used to render the .obj mesh.
* Shading meshes with a Tone Mapping or Cartoon (Toon) shader
  * Custom colors can be specified for the illuminated/shadow colors of each shader.
* Rendering the silhouette, crease, and/or border edges of an object
  * Editable outline thickness and color
  * Option to render edges with miter joins between them for cleaner rendering
* Lighting meshes with either a directional or point light
* Custom background color (including transparent)
* Showing/hiding mesh to see outlines (wireframe mode)
* Rendering to file (png/jpg/bmp/tga)
  * For file formats that support it, a transparent background color will allow you to render an object without a background.
* Saving/Loading Rendering Presets

## Gallery
| | |
|--|--|
|![dragon image](./assets/screenshots/dragon.png) | ![blue dragon image](./assets/screenshots/blue_eyes_white_dragon.png) 
| ![bunny](./assets/screenshots/bunny.png) | ![box](./assets/screenshots/box.png)|
| ![bunny](./assets/screenshots/shape_outlines.png) |![bunny](./assets/screenshots/temp_sphere.png)

## Installing
*(Section partially adapted from an MIT 6.4400 assignment handout)*

This application requires a minimum version of OpenGL 3.3. You may have to update your graphics drivers.

This application uses C++ 11.

To install NPR Studio, first **download the source code of this project as a .zip** and extract it in the directory you want to run it in.

### Installing Build Tools

This project requires CMake, g++, and other building tools for compiling C++. They can be installed for various Operating Systems as follows:

#### Linux
```Shell
sudo apt-get install g++-5 build-essential cmake
```
#### MacOS
1. Install Xcode
2. Install Xcode command-line tools:
```Shell
xcode-select --install
```
3. Use [homebrew](https://brew.sh/) to install CMake:
```Shell
brew update
brew install cmake
```
#### Windows
1. Download the [CMake Windows installer](https://cmake.org/download/).
2. Download newest version of Visual Studio (community version is free at https://visualstudio.microsoft.com/vs/), and install the necessary C++ tools through Visual Studio (or in your preferred way).

### Building
#### Linux / MacOS
Run the following commands at the project's root directory:
```Shell
mkdir build
cd build
cmake ..
make
```
#### Windows
You can use CMake-GUI to generate the build system of your choice:
1. Set the source code location ("Where is the source code") to the project's root directory
2. Set the binary building location ("Where to build the binaries") to the root directory followed by build/.
3. Click Configure. The default configuration should suffice.
4. After configuring, click Generate. Choose the build system you like, i.e Visual Studio 2019.
5. The build system files should **be** in the build directory.

After building the project, you should see an executable in the build directory called "npr_studio". This is the application executable.

## Running
To open an .obj file for rendering, run the application executable in the command line with the following syntax:

```Shell
./npr_studio <filename>
```

If you don't specify a filename, a default mesh will be used for rendering.

## Editing Code
If you add files, be sure to rerun `cmake` running `make`.

## High-Level Code Organization
The main entrypoint of this code is [`main`](./main_code/npr_studio/main.cpp), which calls [`ToonViewerApp`](./main_code/npr_studio/ToonViewerApp.hpp), which sets up the application and GUI.

When importing a mesh, `ToonViewerApp` creates [`OutlineNode`](./main_code/npr_studio/OutlineNode.hpp)s that handle rendering both the mesh itself and rendering its outlines.

The mesh itself is shaded using the [`ToneMappingShader`](./gloo/shaders/ToneMappingShader.hpp) and [`ToonShader`](./gloo/shaders/ToonShader.hpp). 

Mesh outlines are rendered using the [`OutlineShader`](./gloo/shaders/OutlineShader.hpp) and [`MiterOutlineShader`](./gloo/shaders/MiterOutlineShader.hpp).

All shader files have associated GLSL fragment, vertex, and/or geometry shaders found in [`gloo/shaders/glsl`](./gloo/shaders/glsl/).

When rendering edges with miter joins, the list of rendered edges is broken into polylines that are rendered individually by `MiterOutlineShader`. The logic for this is in [`edgeutils`](./main_code/common/edgeutils.hpp).

Lighting is handled using [`SunNode`](./main_code/npr_studio/SunNode.hpp).

Note that GLOO itself is structured similarly to Unity's scripting API.

## Project Writeup
If you're interested in learning a bit behind the algorithms in the project, I've created a writeup for this project, found [here](https://www.mit.edu/~obin/2023-2024/Semester_1/6_4400/final_project/writeup.pdf). Currently, it only touches on the work I've done up to December 13, 2023 (Toon/Tone Shading + Basic (Non-Miter) Outline Rendering).


## Credits
This application is built off of the library `GLOO` (openGL Object-Oriented), a lightweight object-oriented C++ library for interacting with OpenGL that was provided by MIT’s 6.4400 Course Staff (includes essentially all code from the first commit that's not the `/external` directory).

All NPR-specific code (stylized shaders, drawing outlines, application GUI, etc.) was created by Obi_Nnamdi.

### Dependencies
This project uses the libraries GLAD, GLFW, GLM, stb image, and Dear ImGui.