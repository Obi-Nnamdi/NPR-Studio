#ifndef GLOO_POINT_LIGHT_H_
#define GLOO_POINT_LIGHT_H_

#include "LightBase.hpp"

namespace GLOO {
class PointLight : public LightBase {
 public:
  void SetAttenuation(const glm::vec3& attenuation) {
    attenuation_ = attenuation;
  }

  glm::vec3 GetAttenuation() const {
    return attenuation_;
  }

  LightType GetType() const override {
    return LightType::Point;
  }

 private:
  glm::vec3 attenuation_;  // (constant, linear, quadratic)
};
}  // namespace GLOO

#endif
