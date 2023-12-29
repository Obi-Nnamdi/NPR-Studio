#ifndef SUN_NODE_H_
#define SUN_NODE_H_

#include "gloo/SceneNode.hpp"
#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"

namespace GLOO {
class SunNode : public SceneNode {
 public:
  SunNode();

  void Update(double delta_time) override;
  void ToggleLight();  // switches between directional "full" light and point light
  void SetRadius(float radius);
  // Intensity is a value between 0 and 1
  void SetIntensity(float intensity);

 private:
  void UpdateSun(const glm::vec3& eye, const glm::vec3& direction);
  void UpdatePosition(const glm::vec3& position);

  std::shared_ptr<DirectionalLight> directional_light_;
  std::shared_ptr<PointLight> point_light_;
  LightType activated_light_;
  double time_elapsed_;
};
}  // namespace GLOO

#endif
