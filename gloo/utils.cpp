#include "utils.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace GLOO {
std::string project_executable_dir_ = "";
std::string project_shader_dir_ = "";
std::string project_asset_dir_ = "";

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
void UpdateRelativePathsFromConfig() {
  std::string config_path = GetProjectRootDir() + kRootSentinel;

  std::ifstream config(config_path);
  std::string line;
  // Read each line of the config file
  while (std::getline(config, line)) {
    // Discard empty lines or lines with comments (marked with #)
    if (line.rfind("#", /*pos=*/0) == 0) {  // pos=0 limits the search to the prefix
      continue;
    }
    // Get key/value pair
    auto tokens = Split(line, '=');
    if (tokens.size() != 2) {
      throw std::runtime_error("Failure to parse config file.");
    }
    auto key = tokens[0];
    auto value = tokens[1];

    // Update globals based on key
    if (key == "shaders") {
      project_shader_dir_ = value;
      std::cout << "Project shader dir: " << project_shader_dir_ << std::endl;
    } else if (key == "assets") {
      project_asset_dir_ = value;
      std::cout << "Project asset dir: " << project_asset_dir_ << std::endl;
    }
  }
}

std::string GetProjectRootDir() {
  // Set initial searching directory to either the executable path or the "./" relative directory if
  // the executable path isn't found.
  std::string dir = !project_executable_dir_.empty() ? project_executable_dir_ : "./";
  // Recursively going up in directory until finding gloo.cfg
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
  // TODO: Cache value?
}

std::string GetShaderGLSLDir() {
  // Use global shader directory if we have it, or go with default if it's empty
  std::string default_path = "gloo/shaders/glsl/";
  std::string shader_dir = !project_shader_dir_.empty() ? project_shader_dir_ : default_path;
  return GetProjectRootDir() + shader_dir;
}

std::string GetAssetDir() {
  // Use global asset directory if we have it, or go with default
  std::string default_path = "assets/";
  std::string asset_dir = !project_asset_dir_.empty() ? project_asset_dir_ : default_path;
  return GetProjectRootDir() + asset_dir;
}

std::string GetModelDir() { return GetAssetDir() + "models/"; }
std::string GetRenderDir() { return GetAssetDir() + "renders/"; }
std::string GetPresetDir() { return GetAssetDir() + "presets/"; }

}  // namespace GLOO
