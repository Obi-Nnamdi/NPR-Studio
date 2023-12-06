#ifndef GLOO_AMBIENT_LIGHT_H_
#define GLOO_AMBIENT_LIGHT_H_

#include "LightBase.hpp"

namespace GLOO {
class AmbientLight : public LightBase {
  // Use diffuse_color_ as the ambient light color.
 public:
  void SetAmbientColor(const glm::vec3& color) {
    SetDiffuseColor(color);
  }

  glm::vec3 GetAmbientColor() const {
    return GetDiffuseColor();
  }

  LightType GetType() const override {
    return LightType::Ambient;
  }
};
}  // namespace GLOO

#endif
