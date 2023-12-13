#include "SunNode.hpp"

#include "gloo/components/LightComponent.hpp"

#include <glm/gtx/string_cast.hpp>

namespace GLOO {
SunNode::SunNode() : time_elapsed_(0.0f) {
  // Initialize both point and directional light
  point_light_ = std::make_shared<PointLight>();
  point_light_->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light_->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light_->SetAttenuation(glm::vec3(0.15));

  directional_light_ = std::make_shared<DirectionalLight>();
  directional_light_->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  directional_light_->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  CreateComponent<LightComponent>(directional_light_);
  activated_light_ = LightType::Directional;
}

void SunNode::UpdatePosition(const glm::vec3& position) { GetTransform().SetPosition(position); }

void SunNode::ToggleLight() {
  RemoveComponent<LightComponent>();
  // Toggle activated light variable
  activated_light_ =
      activated_light_ == LightType::Directional ? LightType::Point : LightType::Directional;

  // Populate LightComponent with other light
  if (activated_light_ == LightType::Directional) {
    CreateComponent<LightComponent>(directional_light_);
  } else {
    CreateComponent<LightComponent>(point_light_);
  }
}

void SunNode::Update(double delta_time) {
  time_elapsed_ += delta_time;
  glm::vec3 light_dir(2.0f * sinf((float)time_elapsed_ * 1.5f * 0.1f), 5.0f,
                      2.0f * cosf(2 + (float)time_elapsed_ * 1.9f * 0.1f));
  light_dir = glm::normalize(light_dir);
  glm::vec3 eye = 20.0f * light_dir;
  UpdateSun(eye, -light_dir);
  UpdatePosition(2.f * light_dir);  // move point light to same "direction" sun light is in
}

void SunNode::UpdateSun(const glm::vec3& eye, const glm::vec3& direction) {
  directional_light_->SetDirection(glm::normalize(direction));
  auto up_dir =
      glm::normalize(glm::cross(directional_light_->GetDirection(), glm::vec3(0.0f, 0.0f, 1.0f)));
  auto sun_view = glm::lookAt(eye, glm::vec3(0.0f), up_dir);

  auto sun_to_world_mat = glm::inverse(sun_view);
  GetTransform().SetMatrix4x4(sun_to_world_mat);
}
}  // namespace GLOO
