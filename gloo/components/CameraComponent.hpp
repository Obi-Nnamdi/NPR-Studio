#ifndef GLOO_CAMERA_COMPONENT_H_
#define GLOO_CAMERA_COMPONENT_H_

#include "ComponentBase.hpp"

#include <glm/glm.hpp>

namespace GLOO {
class CameraComponent : public ComponentBase {
 public:
  CameraComponent(float fov, float aspect_ratio, float z_near, float z_far);
  glm::mat4 GetProjectionMatrix() const;
  glm::mat4 GetViewMatrix() const;
  void SetAspectRatio(float aspect_ratio) {
    aspect_ratio_ = aspect_ratio;
  }
  void SetViewMatrix(std::unique_ptr<glm::mat4> V) {
    V_ = std::move(V);
  }

 private:
  float fov_;
  float aspect_ratio_;
  float z_near_;
  float z_far_;

  std::unique_ptr<glm::mat4> V_;
};

CREATE_COMPONENT_TRAIT(CameraComponent, ComponentType::Camera);
}  // namespace GLOO

#endif
