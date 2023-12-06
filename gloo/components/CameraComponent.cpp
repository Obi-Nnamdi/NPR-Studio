#include "CameraComponent.hpp"

#include "gloo/utils.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "gloo/SceneNode.hpp"

namespace GLOO {
CameraComponent::CameraComponent(float fov,
                                 float aspect_ratio,
                                 float z_near,
                                 float z_far)
    : fov_(fov),
      aspect_ratio_(aspect_ratio),
      z_near_(z_near),
      z_far_(z_far),
      V_(nullptr) {
}

glm::mat4 CameraComponent::GetProjectionMatrix() const {
  return glm::perspective(fov_ * kPi / 180.f, aspect_ratio_, z_near_, z_far_);
}

glm::mat4 CameraComponent::GetViewMatrix() const {
  if (V_ == nullptr) {
    return glm::inverse(GetNodePtr()->GetTransform().GetLocalToWorldMatrix());
  } else {
    return *V_;
  }
}
}  // namespace GLOO
