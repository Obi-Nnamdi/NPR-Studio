#ifndef MESH_DATA_H_
#define MESH_DATA_H_

#include "gloo/VertexObject.hpp"
#include "gloo/Material.hpp"

namespace GLOO {
struct MeshGroup {
  std::string name;
  size_t start_face_index;
  size_t num_indices;
  std::string material_name;
  std::shared_ptr<Material> material;
};

struct MeshData {
  std::unique_ptr<VertexObject> vertex_obj;
  std::vector<MeshGroup> groups;
};
}  // namespace GLOO

#endif
