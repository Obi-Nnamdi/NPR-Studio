#ifndef GLOO_SCENE_H_
#define GLOO_SCENE_H_

#include <vector>
#include <memory>

#include "SceneNode.hpp"
#include "components/CameraComponent.hpp"

namespace GLOO {

class Scene {
 public:
  Scene(std::unique_ptr<SceneNode> root_node)
      : root_node_(std::move(root_node)), active_camera_ptr_(nullptr) {
  }
  SceneNode& GetRootNode() {
    return *root_node_;
  }
  const SceneNode& GetRootNode() const {
    return *root_node_;
  }
  void ActivateCamera(CameraComponent* camera_ptr) {
    active_camera_ptr_ = camera_ptr;
  }
  CameraComponent* GetActiveCameraPtr() const {
    return active_camera_ptr_;
  }
  void Update(double delta_time);

 private:
  void RecursiveUpdate(SceneNode& node, double delta_time);

  std::unique_ptr<SceneNode> root_node_;
  CameraComponent* active_camera_ptr_;
};
}  // namespace GLOO

#endif
