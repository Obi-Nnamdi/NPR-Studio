#ifndef GLOO_LIGHT_BASE_H_
#define GLOO_LIGHT_BASE_H_

#include <glm/glm.hpp>

namespace GLOO {
enum class LightType {
  Ambient,
  Point,
  Directional,
};

class LightBase {
 public:
  LightBase()
      : diffuse_color_(0.8f, 0.8f, 0.8f), specular_color_(1.0f, 1.0f, 1.0f) {
  }

  virtual ~LightBase() {
  }

  void SetDiffuseColor(const glm::vec3& color) {
    diffuse_color_ = color;
  }

  void SetSpecularColor(const glm::vec3& color) {
    specular_color_ = color;
  }

  glm::vec3 GetDiffuseColor() const {
    return diffuse_color_;
  }

  glm::vec3 GetSpecularColor() const {
    return specular_color_;
  }

  virtual LightType GetType() const = 0;

 private:
  glm::vec3 diffuse_color_;
  glm::vec3 specular_color_;
};
}  // namespace GLOO

#endif
