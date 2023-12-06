#ifndef GLOO_COMPONENT_TYPE_H_
#define GLOO_COMPONENT_TYPE_H_

#include <typeinfo>

#include "gloo/utils.hpp"

namespace GLOO {
enum class ComponentType {
  Undefined = 0,
  Rendering,
  Shading,
  Material,
  Camera,
  Light,
  Tracing,
};

template <typename T>
struct ComponentTrait {
  static ComponentType GetType() {
    static_assert(always_false<T>::value, "Unimplemented component type!");
    return ComponentType::Undefined;
  }
};

#define CREATE_COMPONENT_TRAIT(C, T) \
  template <>                        \
  struct ComponentTrait<C> {         \
    static ComponentType GetType() { \
      return T;                      \
    }                                \
  };

}  // namespace GLOO

#endif
