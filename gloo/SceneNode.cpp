#include "SceneNode.hpp"

#include <glm/gtx/string_cast.hpp>

namespace GLOO {
SceneNode::SceneNode() : transform_(*this), parent_(nullptr), active_(true) {
}

void SceneNode::AddChild(std::unique_ptr<SceneNode> child) {
  child->parent_ = this;
  children_.emplace_back(std::move(child));
}

ComponentBase* SceneNode::GetComponentPtrByType(ComponentType type) const {
  if (IsActive() && component_dict_.count(type)) {
    return component_dict_.at(type).get();
  }
  return nullptr;
}

std::vector<ComponentBase*> SceneNode::GetComponentsPtrInChildrenByType(
    ComponentType type) const {
  std::vector<ComponentBase*> result;
  GatherComponentPtrsRecursivelyByType(type, result);
  return result;
}

void SceneNode::GatherComponentPtrsRecursivelyByType(
    ComponentType type,
    std::vector<ComponentBase*>& result) const {
  ComponentBase* component = GetComponentPtrByType(type);
  if (component != nullptr) {
    result.push_back(component);
  }
  size_t child_count = GetChildrenCount();
  for (size_t i = 0; i < child_count; i++) {
    SceneNode& child = GetChild(i);
    if (child.IsActive()) {
      child.GatherComponentPtrsRecursivelyByType(type, result);
    }
  }
}
}  // namespace GLOO
