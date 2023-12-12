#ifndef OUTLINE_NODE_H_
#define OUTLINE_NODE_H_

#include <functional>
#include <map>
#include <utility>

#include "gloo/SceneNode.hpp"
#include "gloo/VertexObject.hpp"
#include "gloo/shaders/ShaderProgram.hpp"

namespace GLOO {

// Allows us to use pairs as map keys.
// Not a great hash function but good enough for our uses.
// Credit: https://stackoverflow.com/a/20602159/20791863
struct pairhash {
 public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

// Ensures that edge hashing is communative [e.g. (i1, i2) is the same as (i2, i1)].
// Credit: https://stackoverflow.com/q/47394875/20791863
struct KeyEqual {
  template <typename T, typename U>
  bool operator()(const T &a1, const U &a2) const {
    return (a1.first == a2.first && a1.second == a2.second) ||
           (a1.first == a2.second && a1.second == a2.first);
  }
};

// Holds information about the three vertices of a face and its normal
struct Face {
  size_t i1, i2, i3;
  glm::vec3 normal;
};

// An edge is represented as a pair between two indices
using Edge = std::pair<size_t, size_t>;

// Holds rendering information about an edge.
struct EdgeInfo {
  bool is_silhouette, is_crease, is_border;
};

/**
 * Class representing an object shaded with outlines.
 */
class OutlineNode : public SceneNode {
 public:
  OutlineNode();
  void Update(double delta_time) override;

 private:
  void SetupEdgeMaps();
  void ComputeBorderEdges();
  std::unordered_map<Edge, std::vector<Face>, pairhash, KeyEqual> edge_face_map_;
  std::unordered_map<Edge, EdgeInfo, pairhash, KeyEqual> edge_info_map_;

  std::shared_ptr<ShaderProgram> mesh_shader_;
  std::shared_ptr<ShaderProgram> outline_shader_;

  std::shared_ptr<VertexObject> mesh_;
};
}  // namespace GLOO

#endif