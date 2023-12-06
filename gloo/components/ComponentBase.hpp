#ifndef GLOO_COMPONENT_BASE_H_
#define GLOO_COMPONENT_BASE_H_

#include "ComponentType.hpp"

namespace GLOO {
class SceneNode;

class ComponentBase {
 public:
  virtual ~ComponentBase() {
  }
  void SetNodePtr(SceneNode* node_ptr) {
    node_ptr_ = node_ptr;
  }
  SceneNode* GetNodePtr() const {
    return node_ptr_;
  }

 protected:
  SceneNode* node_ptr_;
};
}  // namespace GLOO

#endif
