#ifndef GLOO_BASIC_CAMERA_NODE_H_
#define GLOO_BASIC_CAMERA_NODE_H_

#include "gloo/SceneNode.hpp"

namespace GLOO {
class CameraComponent;

class BasicCameraNode : public SceneNode {
 public:
  BasicCameraNode(float fov = 45.f, float aspect = 0.75f, float speed = 2.0f);
  void Update(double delta_time) override;

 private:
  void UpdateViewport();

  float speed_;
};
}  // namespace GLOO

#endif
