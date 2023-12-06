#include "SunNode.hpp"

#include "gloo/components/LightComponent.hpp"

#include <glm/gtx/string_cast.hpp>

namespace GLOO {
SunNode::SunNode() : time_elapsed_(0.0f) {
  light_ = std::make_shared<DirectionalLight>();
  light_->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  light_->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  CreateComponent<LightComponent>(light_);
}

void SunNode::Update(double delta_time) {
  time_elapsed_ += delta_time;
  glm::vec3 light_dir(2.0f * sinf((float)time_elapsed_ * 1.5f * 0.1f), 5.0f,
                      2.0f * cosf(2 + (float)time_elapsed_ * 1.9f * 0.1f));
  light_dir = glm::normalize(light_dir);
  glm::vec3 eye = 20.0f * light_dir;
  UpdateSun(eye, -light_dir);
}

void SunNode::UpdateSun(const glm::vec3& eye, const glm::vec3& direction) {
  light_->SetDirection(glm::normalize(direction));
  auto up_dir = glm::normalize(
      glm::cross(light_->GetDirection(), glm::vec3(0.0f, 0.0f, 1.0f)));
  auto sun_view = glm::lookAt(eye, glm::vec3(0.0f), up_dir);

  auto sun_to_world_mat = glm::inverse(sun_view);
  GetTransform().SetMatrix4x4(sun_to_world_mat);
}
}  // namespace GLOO
