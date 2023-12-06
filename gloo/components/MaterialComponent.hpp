#ifndef GLOO_MATERIAL_COMPONENT_H_
#define GLOO_MATERIAL_COMPONENT_H_

#include "ComponentBase.hpp"

#include "gloo/Material.hpp"

namespace GLOO {
class MaterialComponent : public ComponentBase {
 public:
  MaterialComponent(std::shared_ptr<Material> material) {
    SetMaterial(std::move(material));
  }

  void SetMaterial(std::shared_ptr<Material> material) {
    material_ = std::move(material);
  }

  Material& GetMaterial() {
    return *material_;
  }

 private:
  std::shared_ptr<Material> material_;
};

CREATE_COMPONENT_TRAIT(MaterialComponent, ComponentType::Material);
}  // namespace GLOO

#endif
