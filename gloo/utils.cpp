#include "utils.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace GLOO {
std::string project_executable_dir_ = "";

std::vector<std::string> Split(const std::string& s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> result;
  while (std::getline(ss, item, delim)) {
    result.emplace_back(std::move(item));
  }
  return result;
}

void _CheckOpenGLError(const char* stmt, const char* fname, int line) {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    fprintf(stderr, "OpenGL error %08x, at %s:%i - for %s\n", err, fname, line,
            stmt);
    err = glGetError();
  }
}

float ToRadian(float angle) {
  return angle / 180.0f * kPi;
}

std::string GetBasePath(const std::string& path) {
  size_t last_sep = path.find_last_of("\\/");
  std::string base_path;
  if (last_sep == std::string::npos) {
    base_path = "";
  } else {
    base_path = path.substr(0, last_sep + 1);
  }
  return base_path;
}

const std::string kRootSentinel = "gloo.cfg";
const int kMaxDepth = 20;

void SetProjectExecutableDir(std::string path) { project_executable_dir_ = GetBasePath(path); }

std::string GetProjectRootDir() {
  // Set initial searching directory to either the executable path or the "./" relative directory if
  // the executable path isn't found.
  std::string dir = project_executable_dir_ != "" ? project_executable_dir_ : "./";
  // Recursively going up in directory until finding .gloo_config
  for (int i = 0; i < kMaxDepth; i++) {
    std::ifstream ifs(dir + kRootSentinel);
    if (ifs.good()) {
      return dir;
    }
    dir = dir + "../";
  }

  throw std::runtime_error("Cannot locate project root directory with a " +
                           kRootSentinel + " file after " +
                           std::to_string(kMaxDepth) + " levels!");
}

std::string GetShaderGLSLDir() {
  // TODO: Read from gloo.cfg file itself to get the shader directory
  return GetProjectRootDir() + "gloo/shaders/glsl/";
}

std::string GetAssetDir() {
  // TODO: Read from gloo.cfg file itself to get the asset directory
  return GetProjectRootDir() + "assets/";
}

std::string GetModelDir() { return GetAssetDir() + "models/"; }
std::string GetRenderDir() { return GetAssetDir() + "renders/"; }
std::string GetPresetDir() { return GetAssetDir() + "presets/"; }

}  // namespace GLOO
