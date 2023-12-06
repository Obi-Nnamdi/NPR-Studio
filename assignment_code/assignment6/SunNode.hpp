#ifndef SUN_NODE_H_
#define SUN_NODE_H_

#include "gloo/SceneNode.hpp"

#include "gloo/lights/DirectionalLight.hpp"

namespace GLOO {
class SunNode : public SceneNode {
 public:
  SunNode();

  void Update(double delta_time) override;

 private:
  void UpdateSun(const glm::vec3& eye, const glm::vec3& direction);

  std::shared_ptr<DirectionalLight> light_;
  double time_elapsed_;
};
}  // namespace GLOO

#endif
