#include "BasicCameraNode.hpp"

#include <iostream>

#include <glm/gtc/quaternion.hpp>

#include "gloo/components/CameraComponent.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
BasicCameraNode::BasicCameraNode(float fov, float aspect, float speed)
    : SceneNode(), speed_(speed) {
  auto camera = make_unique<CameraComponent>(fov, aspect, 0.1f, 100.f);
  AddComponent(std::move(camera));
}

void BasicCameraNode::Update(double delta_time) {
  UpdateViewport();

  float delta_dist = speed_ * static_cast<float>(delta_time);
  glm::vec3 old_position = GetTransform().GetPosition();
  glm::vec3 new_position = old_position;
  if (InputManager::GetInstance().IsKeyPressed('W')) {
    new_position += delta_dist * GetTransform().GetForwardDirection();
  }
  if (InputManager::GetInstance().IsKeyPressed('S')) {
    new_position -= delta_dist * GetTransform().GetForwardDirection();
  }
  if (InputManager::GetInstance().IsKeyPressed('A')) {
    new_position -= delta_dist * GetTransform().GetRightDirection();
  }
  if (InputManager::GetInstance().IsKeyPressed('D')) {
    new_position += delta_dist * GetTransform().GetRightDirection();
  }
  GetTransform().SetPosition(new_position);
}

void BasicCameraNode::UpdateViewport() {
  glm::ivec2 window_size = InputManager::GetInstance().GetWindowSize();
  float aspect_ratio =
      static_cast<float>(window_size.x) / static_cast<float>(window_size.y);
  GetComponentPtr<CameraComponent>()->SetAspectRatio(aspect_ratio);
}
}  // namespace GLOO
