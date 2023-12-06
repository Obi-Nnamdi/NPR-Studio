#ifndef GLOO_LIGHT_COMPONENT_H_
#define GLOO_LIGHT_COMPONENT_H_

#include "ComponentBase.hpp"

#include <glm/glm.hpp>

#include "gloo/lights/LightBase.hpp"

namespace GLOO {
class LightComponent : public ComponentBase {
 public:
  LightComponent(std::shared_ptr<LightBase> light) : light_(std::move(light)) {
  }
  LightBase* GetLightPtr() const {
    return light_.get();
  }

  bool CanCastShadow() const {
    return light_->GetType() == LightType::Directional;
  }

 private:
  std::shared_ptr<LightBase> light_;
};

CREATE_COMPONENT_TRAIT(LightComponent, ComponentType::Light);
}  // namespace GLOO

#endif
