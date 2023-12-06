#ifndef GLOO_SHADING_COMPONENT_H_
#define GLOO_SHADING_COMPONENT_H_

#include "ComponentBase.hpp"

#include <memory>

#include "gloo/shaders/ShaderProgram.hpp"
#include "gloo/alias_types.hpp"

namespace GLOO {
class ShadingComponent : public ComponentBase {
 public:
  ShadingComponent(std::shared_ptr<ShaderProgram> shader)
      : shader_(std::move(shader)) {
  }
  ShaderProgram* GetShaderPtr() {
    return shader_.get();
  }

 private:
  std::shared_ptr<ShaderProgram> shader_;
};

CREATE_COMPONENT_TRAIT(ShadingComponent, ComponentType::Shading);
}  // namespace GLOO

#endif
