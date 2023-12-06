#include "Scene.hpp"

namespace GLOO {

void Scene::Update(double delta_time) {
  RecursiveUpdate(*root_node_, delta_time);
}

void Scene::RecursiveUpdate(SceneNode& node, double delta_time) {
  node.Update(delta_time);
  size_t child_count = node.GetChildrenCount();
  for (size_t i = 0; i < child_count; i++) {
    RecursiveUpdate(node.GetChild(i), delta_time);
  }
}
}  // namespace GLOO
