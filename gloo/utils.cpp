#include "utils.hpp"

#include <cstdio>

#include <iostream>
#include <fstream>
#include <stdexcept>

namespace GLOO {
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

std::string GetProjectRootDir() {
  // Recursively going up in directory until finding .gloo_config
  std::string dir = "./";
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
  return GetProjectRootDir() + "gloo/shaders/glsl/";
}

std::string GetAssetDir() {
  return GetProjectRootDir() + "assets/";
}

}  // namespace GLOO
