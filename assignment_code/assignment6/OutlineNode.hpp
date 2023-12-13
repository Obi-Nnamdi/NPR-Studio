#ifndef OUTLINE_NODE_H_
#define OUTLINE_NODE_H_

#include <functional>
#include <map>
#include <utility>

#include "gloo/Scene.hpp"
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

enum ToonShadingType { TOON, TONE_MAPPING };

/**
 * Class representing an object shaded with outlines.
 */
class OutlineNode : public SceneNode {
 public:
  //  Defaults to tone mapping shader if shader not specified.
  //   Uses basic cylinder if mesh not specified.
  OutlineNode(const Scene *scene, const std::shared_ptr<VertexObject> mesh = nullptr,
              const std::shared_ptr<ShaderProgram> mesh_shader = nullptr);
  void Update(double delta_time) override;
  // Change shader applied to outline mesh
  void ChangeMeshShader(std::shared_ptr<ShaderProgram> shader);
  void ChangeMeshShader(ToonShadingType shaderType);
  //   TODO: functions for turning edge types on and off

 private:
  void SetupEdgeMaps();
  void ComputeBorderEdges();
  void ComputeCreaseEdges();
  void ComputeSilhouetteEdges();
  void SetOutlineMesh();
  void DoRenderSetup(std::shared_ptr<ShaderProgram> mesh_shader = nullptr);
  // Modify outline_mesh_ to give it indices corresponding only to edges of the types that are true.
  void RenderEdges(bool silhouette = true, bool border = true, bool crease = true);
  std::unordered_map<Edge, std::vector<Face>, pairhash, KeyEqual> edge_face_map_;
  std::unordered_map<Edge, EdgeInfo, pairhash, KeyEqual> edge_info_map_;

  std::shared_ptr<ShaderProgram> mesh_shader_;
  std::shared_ptr<ShaderProgram> outline_shader_;

  std::shared_ptr<VertexObject> mesh_;
  std::shared_ptr<VertexObject> outline_mesh_;

  SceneNode *mesh_node_;

  const float line_bias_ = 0.005;
  float crease_threshold_ = glm::radians(10.f);
  const Scene *parent_scene_;
};
}  // namespace GLOO

#endif