#ifndef GLOO_DIRECTIONAL_LIGHT_H_
#define GLOO_DIRECTIONAL_LIGHT_H_

#include "LightBase.hpp"

namespace GLOO {
class DirectionalLight : public LightBase {
 public:
  void SetDirection(const glm::vec3& direction) {
    direction_ = glm::normalize(direction);
  }

  glm::vec3 GetDirection() const {
    return direction_;
  }

  LightType GetType() const override {
    return LightType::Directional;
  }

 private:
  glm::vec3 direction_;
};
}  // namespace GLOO

#endif


