#ifndef GLOO_SCENE_NODE_H_
#define GLOO_SCENE_NODE_H_

#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>
#include <typeinfo>
#include <stdexcept>

#include <glm/vec3.hpp>

#include "components/ComponentBase.hpp"
#include "components/ComponentType.hpp"
#include "Transform.hpp"

namespace GLOO {
class SceneNode {
 public:
  SceneNode();
  virtual ~SceneNode() {
  }

  size_t GetChildrenCount() const {
    return children_.size();
  }

  SceneNode& GetChild(size_t index) const {
    return *children_.at(index);
  }

  SceneNode* GetParentPtr() const {
    return parent_;
  }

  void AddChild(std::unique_ptr<SceneNode> child);

  template <class T>
  void AddComponent(std::unique_ptr<T> component) {
    component->SetNodePtr(this);
    component_dict_[ComponentTrait<T>::GetType()] = std::move(component);
  }

  template <class T>
  bool RemoveComponent() {
    auto itr = component_dict_.find(ComponentTrait<T>::GetType());
    if (itr != component_dict_.end()) {
      component_dict_.erase(itr);
      return true;
    }
    return false;
  }

  template <class T, typename... Args>
  T& CreateComponent(Args&&... args) {
    AddComponent(make_unique<T>(std::forward<Args>(args)...));
    return *GetComponentPtr<T>();
  }

  template <class T>
  T* GetComponentPtr() const {
    // Returns pointer since the component may not exist.
    return static_cast<T*>(GetComponentPtrByType(ComponentTrait<T>::GetType()));
  }

  template <class T>
  std::vector<T*> GetComponentPtrsInChildren() const {
    std::vector<T*> result;
    std::vector<ComponentBase*> result_raw =
        GetComponentsPtrInChildrenByType(ComponentTrait<T>::GetType());
    for (ComponentBase* c : result_raw) {
      result.push_back(static_cast<T*>(c));
    }
    return result;
  }

  Transform& GetTransform() {
    return transform_;
  }

  // Const overloading.
  const Transform& GetTransform() const {
    return transform_;
  }

  bool IsActive() const {
    return active_;
  }
  void SetActive(bool new_state) {
    active_ = new_state;
  }

  virtual void Update(double delta_time) {
  }

 private:
  ComponentBase* GetComponentPtrByType(ComponentType type) const;
  std::vector<ComponentBase*> GetComponentsPtrInChildrenByType(
      ComponentType type) const;
  void GatherComponentPtrsRecursivelyByType(
      ComponentType type,
      std::vector<ComponentBase*>& result) const;

  Transform transform_;
  std::unordered_map<ComponentType,
                     std::unique_ptr<ComponentBase>,
                     EnumClassHash>
      component_dict_;
  std::vector<std::unique_ptr<SceneNode>> children_;
  SceneNode* parent_;
  bool active_;
};
}  // namespace GLOO

#endif
