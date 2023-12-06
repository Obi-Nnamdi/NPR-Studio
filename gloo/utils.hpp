#ifndef GLOO_UTILS_H_
#define GLOO_UTILS_H_

#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include <glad/glad.h>

namespace GLOO {
void _CheckOpenGLError(const char* stmt, const char* fname, int line);

/*
 * https://stackoverflow.com/questions/11256470/define-a-macro-to-facilitate-opengl-command-debugging
 * */
#ifndef NDEBUG
#define GL_CHECK(stmt)                            \
  do {                                            \
    stmt;                                         \
    _CheckOpenGLError(#stmt, __FILE__, __LINE__); \
  } while (0)
#define GL_CHECK_ERROR() \
  _CheckOpenGLError("GL_CHECK_ERROR", __FILE__, __LINE__);
#else
#define GL_CHECK(stmt) stmt
#define GL_CHECK_ERROR()
#endif

// Force compile error in templates without getting warnings.
template <typename T>
struct always_false {
  enum { value = false };
};

const float kPi = std::atan(1.0f) * 4;
const float kGravity = 9.81f;

float ToRadian(float angle);

// Emulating python's split operation.
std::vector<std::string> Split(const std::string& s, char delim);

// Get the base directory of a path (including the last '/' or '\').
std::string GetBasePath(const std::string& path);

// Helpers for managing paths.
std::string GetProjectRootDir();
std::string GetShaderGLSLDir();
std::string GetAssetDir();

// C++11 does not have make_unique sadly; it appeared in C++14.
// MSVC already has make_unique defined.
#ifdef _MSC_VER
#define make_unique std::make_unique
#else
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

// Another one of C++11 shenanigans that was fixed later:
// https://stackoverflow.com/questions/18837857/cant-use-enum-class-as-unordered-map-key
struct EnumClassHash {
  template <typename T>
  std::size_t operator()(T t) const {
    return static_cast<std::size_t>(t);
  }
};

// Silence compiler warning for unused variables that might be used in
// student's implementation or in future assignments.
#define UNUSED(expr) \
  do {               \
    (void)(expr);    \
  } while (0)
}  // namespace GLOO

#endif
